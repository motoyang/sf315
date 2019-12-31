// -- adapter.rs --

use {
    super::{
        drop_guard::DropGuard,
        servant::{Record, ServantRegister, ServantResult},
    },
    async_std::{net::TcpStream, prelude::*, sync::Mutex, task},
    codec::RecordCodec,
    futures::{
        channel::mpsc::{unbounded, UnboundedSender},
        pin_mut, select,
        sink::SinkExt,
        FutureExt as _,
    },
    futures_codec::{FramedRead, FramedWrite},
    log::{info, warn},
    std::{collections::HashMap, net::SocketAddr},
};

// --

lazy_static! {
    static ref ADAPTER_REGISTER: AdapterRegister = AdapterRegister(Mutex::new(_Register {
        passcode: 238,
        id: 0,
        accept_tx: None,
        senders: HashMap::new(),
    }));
}

struct _Register {
    passcode: usize,
    id: usize,
    accept_tx: Option<UnboundedSender<()>>,
    senders: HashMap<SocketAddr, UnboundedSender<Record>>,
}

pub struct AdapterRegister(Mutex<_Register>);
impl AdapterRegister {
    pub fn instance() -> &'static Self {
        &ADAPTER_REGISTER
    }
    pub async fn clean(&self, passcode: usize) {
        let mut g = self.0.lock().await;
        if passcode == g.passcode {
            g.accept_tx.take();
            g.senders.clear();
        }
    }
    pub async fn set_accept(&self, tx: UnboundedSender<()>) {
        let mut g = self.0.lock().await;
        g.accept_tx = Some(tx);
    }
    pub async fn insert(&self, addr: SocketAddr, tx: UnboundedSender<Record>) {
        let mut g = self.0.lock().await;
        g.senders.insert(addr, tx);
    }
    pub async fn remove(&self, addr: &SocketAddr) {
        let mut g = self.0.lock().await;
        g.senders.remove(addr);
    }
    pub async fn send(&self, msg: Vec<u8>) {
        let mut g = self.0.lock().await;
        g.id += 1;
        let notice = Record::Notice { id: g.id, msg };
        let mut values = g.senders.values();
        while let Some(mut s) = values.next() {
            s.send(notice.clone())
                .await
                .unwrap_or_else(|e| warn!("{}", e.to_string()));
        }
    }
}

// --

pub struct Adapter;

impl Adapter {
    pub fn new() -> Self {
        Self
    }
    pub async fn run(self, stream: TcpStream) -> std::io::Result<()> {
        #[derive(Debug)]
        enum SelectedValue {
            ReadNone,
            WriteNone,
            Read(Record),
            Write(Record),
        };

        let addr = stream.peer_addr().unwrap();
        info!("connected from {}", &addr);
        let (reader, writer) = &mut (&stream, &stream);
        let read_framed = FramedRead::new(reader, RecordCodec::<u32, Record>::default());
        let mut write_framed = FramedWrite::new(writer, RecordCodec::<u32, Record>::default());

        let (tx, rx) = unbounded();
        AdapterRegister::instance().insert(addr, tx).await;

        let _adapter_clean = DropGuard::new(addr, |a| {
            task::block_on(async move {
                info!("adapter from {} quit.", &addr);
                AdapterRegister::instance().remove(&a).await;
            });
        });

        pin_mut!(read_framed, rx);
        loop {
            let value = select! {
                from_terminal = read_framed.next().fuse() => match from_terminal {
                    Some(record) => SelectedValue::Read(record?),
                    None => SelectedValue::ReadNone,
                },
                to_terminal = rx.next().fuse() => match to_terminal {
                    Some(record) => SelectedValue::Write(record),
                    None => SelectedValue::WriteNone,
                },
            };
            let sr = ServantRegister::instance();
            match value {
                SelectedValue::Read(record) => match record {
                    Record::Report { id, oid, msg } => {
                        let _id = id;
                        if let Some(servant) = sr.find_report_servant(&oid) {
                            servant.lock().unwrap().serve(msg);
                        } else {
                            warn!("{} dosen't exist.", &oid);
                        }
                    }
                    Record::Request { id, oid, req } => {
                        let ret: ServantResult<Vec<u8>> = if let Some(oid) = &oid {
                            if let Some(servant) = sr.find_servant(oid) {
                                Ok(servant.lock().unwrap().serve(req))
                            } else {
                                Err(format!("{} dosen't exist.", &oid).into())
                            }
                        } else {
                            let query = sr.query_servant();
                            let mut q = query.lock().unwrap();
                            Ok(q.serve(req))
                        };
                        match bincode::serialize(&ret) {
                            Ok(ret) => {
                                let record = Record::Response { id, oid, ret };
                                write_framed.send(record).await?;
                            }
                            Err(e) => warn!("{}", e.to_string()),
                        }
                    }
                    Record::Response { .. } => unreachable!(),
                    Record::Notice { .. } => unreachable!(),
                },
                SelectedValue::Write(record) => {
                    write_framed.send(record).await?;
                }
                _ => {
                    info!(
                        "loop break due to SelectValue({:?}). peer address: {}",
                        &value, &addr
                    );
                    break;
                }
            }
        }

        Ok(())
    }
}
