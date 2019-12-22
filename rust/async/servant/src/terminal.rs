// -- terminal.rs --

use {
    super::{
        drop_guard::DropGuard,
        servant::{Oid, OutOfBandRequest, PushMessage, Record, ServantResult},
    },
    async_std::{
        net::{TcpStream, ToSocketAddrs},
        prelude::*,
        sync::{Arc, Mutex},
        task,
    },
    bytes::Bytes,
    codec::LengthCodec,
    futures::{
        channel::mpsc::{unbounded, UnboundedSender},
        pin_mut, select,
        sink::SinkExt,
        FutureExt as _,
    },
    futures_codec::{FramedRead, FramedWrite},
    log::{error, info, trace, warn},
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
    m: StdMutex<Option<Vec<u8>>>,
    cv: Condvar,
}
type Token = Arc<_Token>;
type TokenMap = HashMap<usize, Token>;
type TokenPool = Vec<Token>;
#[derive(Debug, Clone)]
struct _Terminal {
    max_req_id: usize,
    req_id: usize,
    push_id: usize,
    sender: Option<Tx>,
    pool: TokenPool,
    map: TokenMap,
}

#[derive(Debug, Clone)]
pub struct Terminal(Arc<Mutex<_Terminal>>);
impl Terminal {
    pub fn new(max_req_id: usize) -> Self {
        let mut t = _Terminal {
            max_req_id,
            req_id: 0,
            push_id: 0,
            sender: None,
            pool: TokenPool::new(),
            map: TokenMap::new(),
        };
        for _ in 0..t.max_req_id {
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
    pub async fn push(&mut self, msg: PushMessage) -> ServantResult<()> {
        let mut g = self.0.lock().await;
        g.push_id += 1;
        if let Some(mut tx) = g.sender.as_ref() {
            let record = Record::Push { id: g.push_id, msg };
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
        let (mut tx, index, request) = {
            let mut g = self.0.lock().await;
            if let Some(req) = g.pool.pop() {
                g.req_id += 1;
                let id = g.req_id;
                g.map.insert(id, req.clone());
                let tx = g.sender.as_ref().unwrap().clone();
                (tx, g.req_id, req)
            } else {
                return Err("request pool is empty.".into());
            }
        };

        let ret = match request.m.lock() {
            Ok(m) => {
                assert_eq!(*m, None);
                let record = Record::Invoke {
                    id: index,
                    oid,
                    req,
                };
                if let Err(e) = tx.send(record).await {
                    Err(e.to_string().into())
                } else {
                    match request.cv.wait_timeout(m, Duration::from_secs(5)) {
                        Ok(mut r) => {
                            if !r.1.timed_out() {
                                Ok(r.0.take().unwrap())
                            } else {
                                Err("timed_out.".into())
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
            if let Some(req) = g.map.remove(&index) {
                g.pool.push(req);
            }
        }
        ret
    }
    async fn received(&mut self, record: Record) -> ServantResult<()> {
        match record {
            Record::Push { id, msg } => {
                dbg!((id, msg));
            }
            Record::Return { id, oid, ret } => {
                let _oid = oid;
                let request = {
                    let mut g = self.0.lock().await;
                    g.map.remove(&id)
                };
                if let Some(request) = request {
                    {
                        let mut g = request.m.lock().unwrap();
                        assert_eq!(*g, None);
                        g.replace(ret);
                        request.cv.notify_one();
                    }
                    let mut g = self.0.lock().await;
                    g.pool.push(request);
                } else {
                    return Err(format!("can't find id: {} in request map.", id).into());
                }
            }
            Record::Invoke { .. } => unreachable!(),
        }
        Ok(())
    }
    pub async fn run(mut self, addr: impl ToSocketAddrs) -> ServantResult<()> {
        #[derive(Debug)]
        enum SelectedValue {
            ReadNone,
            ReadError(std::io::Error),
            Read(Bytes),
            WriteNone,
            Write(Record),
        }

        let stream = match TcpStream::connect(addr).await {
            Ok(s) => s,
            Err(e) => {
                warn!("connect err: {:?}", e);
                return Err(e.to_string().into());
            }
        };
        let (reader, writer) = (&stream, &stream);
        let read_framed = FramedRead::new(reader, LengthCodec::<u32>::default());
        let mut write_framed = FramedWrite::new(writer, LengthCodec::<u32>::default());

        let (tx, mut rx) = unbounded();
        self.set_tx(Some(tx)).await;
        let _terminal_clean = DropGuard::new(self.clone(), |mut t| {
            task::block_on(async move {
                info!("terminal quit.");
                t.clean().await;
            });
        });

        pin_mut!(read_framed);
        loop {
            let value = select! {
                read_msg = read_framed.next().fuse() => match read_msg {
                    Some(msg) => match msg {
                        Ok(msg) => SelectedValue::Read(msg),
                        Err(e) => SelectedValue::ReadError(e),
                    },
                    None => SelectedValue::ReadNone,
                },
                send_msg = rx.next().fuse() => match send_msg {
                    Some(record) => SelectedValue::Write(record),
                    None => SelectedValue::WriteNone,
                },
            };

            match value {
                SelectedValue::Read(msg) => match bincode::deserialize(&msg) {
                    Ok(record) => {
                        self.received(record)
                            .await
                            .unwrap_or_else(|e| error!("{}", e.to_string()));
                    }
                    Err(e) => error!("{}", e.to_string()),
                },
                SelectedValue::Write(record) => {
                    let v = bincode::serialize(&record).unwrap();
                    if let Err(e) = write_framed.send(Bytes::copy_from_slice(&v)).await {
                        warn!("write to remote error: {:?}", e);
                        return Err(e.to_string().into());
                    }
                }
                SelectedValue::ReadError(e) => {
                    warn!("ReadError: {:?}", e);
                    break;
                }
                _ => {
                    trace!("selected value: {:?}", value);
                    break;
                }
            }
        }
        Ok(())
    }
    pub fn proxy<T, F>(&self, name: String, f: F) -> T
    where
        F: Fn(String, Terminal) -> T,
    {
        f(name, self.clone())
    }
}

pub struct OutOfBandProxy(Terminal);
impl OutOfBandProxy {
    pub fn new(name: String, t: Terminal) -> Self {
        let _oid = Oid::new(name, "OutOfBand".to_string());
        Self(t.clone())
    }
    #[allow(unused)]
    pub async fn export(&mut self) -> ServantResult<Vec<Oid>> {
        let request = OutOfBandRequest::Export {};
        let response = self
            .0
            .invoke(None, bincode::serialize(&request).unwrap())
            .await?;
        Ok(bincode::deserialize(&response).unwrap())
    }
}
