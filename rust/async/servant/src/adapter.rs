// -- adapter.rs --

use {
    super::{
        drop_guard::DropGuard,
        notifier::Notifier,
        servant::{Record, ServantRegister},
    },
    async_std::{net::TcpStream, prelude::*, task},
    bytes::Bytes,
    codec::LengthCodec,
    futures::{channel::mpsc::unbounded, pin_mut, select, sink::SinkExt, FutureExt as _},
    futures_codec::{FramedRead, FramedWrite},
    log::{error, info, warn},
};

// --

pub struct Adapter;

impl Adapter {
    pub fn new() -> Self {
        Self
    }
    pub async fn run(self, stream: TcpStream) -> std::io::Result<()> {
        enum SelectedValue {
            None,
            Read(Bytes),
            Write(Record),
        };

        let addr = stream.peer_addr().unwrap();
        let (reader, writer) = &mut (&stream, &stream);
        let read_framed = FramedRead::new(reader, LengthCodec::<u32>::default());
        let mut write_framed = FramedWrite::new(writer, LengthCodec::<u32>::default());
        let (tx, rx) = unbounded();

        let notifier = Notifier::instance();
        notifier.insert(addr, tx).await;

        let _adapter_clean = DropGuard::new(addr, |a| {
            task::block_on(async move {
                info!("adapter for ({}) quit.", &addr);
                Notifier::instance().remove(&a).await;
            });
        });

        pin_mut!(read_framed, rx);
        loop {
            let value = select! {
                from_terminal = read_framed.next().fuse() => match from_terminal {
                    Some(msg) => SelectedValue::Read(msg?),
                    None => SelectedValue::None,
                },
                to_terminal = rx.next().fuse() => match to_terminal {
                    Some(record) => SelectedValue::Write(record),
                    None => SelectedValue::None,
                },
            };
            let sr = ServantRegister::instance();
            match value {
                SelectedValue::Read(msg) => match bincode::deserialize(&msg) {
                    Ok(record) => match record {
                        Record::Report { id, oid, msg } => {
                            let _id = id;
                            if let Some(servant) = sr.find_report_servant(&oid) {
                                servant.lock().unwrap().serve(msg);
                            } else {
                                warn!("can't find oid {} in register.", oid);
                            }
                        }
                        Record::Request { id, oid, req } => {
                            let ret = if let Some(oid) = &oid {
                                if let Some(servant) = sr.find_servant(oid) {
                                    servant.lock().unwrap().serve(req)
                                } else {
                                    Err(format!("can't find oid {} in register.", oid).into())
                                }
                            } else {
                                let query = sr.query_servant();
                                let mut q = query.lock().unwrap();
                                q.serve(req)
                            };
                            match ret {
                                Ok(ret) => {
                                    let record = Record::Response { id, oid, ret };
                                    let v = bincode::serialize(&record).unwrap();
                                    write_framed.send(Bytes::copy_from_slice(&v)).await?;
                                }
                                Err(e) => warn!("{}", e.to_string()),
                            }
                        }
                        Record::Response { .. } => unreachable!(),
                        Record::Notice { .. } => unreachable!(),
                    },
                    Err(e) => error!("{}", e.to_string()),
                },
                SelectedValue::Write(record) => {
                    let v = bincode::serialize(&record).unwrap();
                    write_framed.send(Bytes::copy_from_slice(&v)).await?;
                }
                SelectedValue::None => break,
            }
        }

        Ok(())
    }
}
