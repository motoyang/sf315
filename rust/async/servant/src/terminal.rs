// -- terminal.rs --

use {
    super::{
        drop_guard::DropGuard,
        servant::{NotifyServant, Oid, Record, ServantResult},
    },
    async_std::{
        net::{TcpStream, ToSocketAddrs},
        prelude::*,
        sync::{Arc, Mutex},
        task,
    },
    codec::RecordCodec,
    futures::{
        channel::mpsc::{unbounded, UnboundedSender},
        pin_mut, select,
        sink::SinkExt,
        FutureExt as _,
    },
    futures_codec::{FramedRead, FramedWrite},
    log::{info, warn},
    std::{
        collections::HashMap,
        sync::{Condvar, Mutex as StdMutex},
        time::Duration,
    },
};

// --

type Tx = UnboundedSender<Record>;
#[derive(Debug)]
struct _Token {
    m: StdMutex<Option<ServantResult<Vec<u8>>>>,
    cv: Condvar,
}
type Token = Arc<_Token>;
type TokenMap = HashMap<usize, Token>;
type TokenPool = Vec<Token>;
type NotifyServantEntry = Box<dyn NotifyServant + Send>;

struct _Terminal {
    req_id: usize,
    report_id: usize,
    sender: Option<Tx>,
    pool: TokenPool,
    map: TokenMap,
    receiver: NotifyServantEntry,
}

#[derive(Clone)]
pub struct Terminal(Arc<Mutex<_Terminal>>);
impl Terminal {
    pub fn new(max_req_id: usize, receiver: NotifyServantEntry) -> Self {
        let mut t = _Terminal {
            req_id: 0,
            report_id: 0,
            sender: None,
            pool: TokenPool::new(),
            map: TokenMap::new(),
            receiver,
        };
        for _ in 0..max_req_id {
            let r = _Token {
                m: StdMutex::new(None),
                cv: Condvar::default(),
            };
            t.pool.push(Arc::new(r));
        }
        Self(Arc::new(Mutex::new(t)))
    }
    pub async fn clean(&mut self) {
        let mut g = self.0.lock().await;
        g.sender.take();
    }
    async fn set_tx(&mut self, tx: Option<Tx>) {
        let mut g = self.0.lock().await;
        g.sender = tx;
    }
    pub async fn report(&mut self, oid: Oid, msg: Vec<u8>) -> ServantResult<()> {
        let mut g = self.0.lock().await;
        g.report_id += 1;
        if let Some(mut tx) = g.sender.as_ref() {
            let record = Record::Report {
                id: g.report_id,
                oid,
                msg,
            };
            if let Err(e) = tx.send(record).await {
                Err(e.to_string().into())
            } else {
                Ok(())
            }
        } else {
            Err("sender is none.".into())
        }
    }
    pub async fn invoke(&mut self, oid: Option<Oid>, req: Vec<u8>) -> ServantResult<Vec<u8>> {
        let (mut tx, index, token) = {
            let mut g = self.0.lock().await;
            if let Some(tok) = g.pool.pop() {
                g.req_id += 1;
                let id = g.req_id;
                g.map.insert(id, tok.clone());
                let tx = if let Some(tx) = g.sender.as_ref() {
                    tx.clone()
                } else {
                    return Err("sender is none.".into());
                };
                (tx, id, tok)
            } else {
                return Err("token pool is empty.".into());
            }
        };
        let ret = match token.m.lock() {
            Ok(m) => {
                let record = Record::Request {
                    id: index,
                    oid,
                    req,
                };
                if let Err(e) = tx.send(record).await {
                    Err(e.to_string().into())
                } else {
                    match token.cv.wait_timeout(m, Duration::from_secs(5)) {
                        Ok(mut r) => {
                            if r.1.timed_out() {
                                Err("timed_out.".into())
                            } else {
                                r.0.take().unwrap()
                            }
                        }
                        Err(e) => Err(e.to_string().into()),
                    }
                }
            }
            Err(e) => Err(e.to_string().into()),
        };
        {
            let mut g = self.0.lock().await;
            g.map.remove(&index);
            g.pool.push(token);
        }
        ret
    }
    async fn received(&mut self, record: Record) {
        match record {
            Record::Notice { id, msg } => {
                let _id = id;
                let mut g = self.0.lock().await;
                g.receiver.serve(msg);
            }
            Record::Response { id, oid, ret } => {
                let _oid = oid;
                let token = {
                    let mut g = self.0.lock().await;
                    g.map.remove(&id)
                };
                if let Some(token) = token {
                    let ret = match bincode::deserialize(&ret) {
                        Ok(ret) => ret,
                        Err(e) => Err(e.to_string().into())
                    };
                    let mut g = token.m.lock().unwrap();
                    g.replace(ret);
                    token.cv.notify_one();
                } else {
                    warn!("can't find id: {} in token map.", id);
                }
            }
            Record::Report { .. } => unreachable!(),
            Record::Request { .. } => unreachable!(),
        }
    }
    pub async fn run(mut self, addr: impl ToSocketAddrs) -> std::io::Result<()> {
        #[derive(Debug)]
        enum SelectedValue {
            ReadNone,
            WriteNone,
            Read(Record),
            Write(Record),
        }

        let stream = TcpStream::connect(addr).await?;
        let (reader, writer) = (&stream, &stream);
        let read_framed = FramedRead::new(reader, RecordCodec::<u32, Record>::default());
        let mut write_framed = FramedWrite::new(writer, RecordCodec::<u32, Record>::default());

        let (tx, rx) = unbounded();
        self.set_tx(Some(tx)).await;
        let _terminal_clean = DropGuard::new(self.clone(), |mut t| {
            task::block_on(async move {
                info!("terminal quit.");
                t.clean().await;
            });
        });

        pin_mut!(read_framed, rx);
        loop {
            let value = select! {
                from_adapter = read_framed.next().fuse() => match from_adapter {
                    Some(record) => SelectedValue::Read(record?),
                    None => SelectedValue::ReadNone,
                },
                to_adapter = rx.next().fuse() => match to_adapter {
                    Some(record) => SelectedValue::Write(record),
                    None => SelectedValue::WriteNone,
                },
            };

            match value {
                SelectedValue::Read(record) => self.received(record).await,
                SelectedValue::Write(record) => write_framed.send(record).await?,
                _ => {
                    info!("loop break due to SelectedValue: {:?}", value);
                    break;
                }
            }
        }
        Ok(())
    }
    pub fn proxy<T, F>(&self, name: String, f: F) -> T
    where
        F: Fn(String, &Terminal) -> T,
    {
        f(name, self)
    }
}
