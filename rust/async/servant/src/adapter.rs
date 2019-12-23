// -- adapter.rs --

use {
    super::servant::{
        Oid, PushMessage, Record, ServantRegister,
        ServantResult,
    },
    async_std::{
        net::TcpStream,
        prelude::*,
        sync::{Arc, Mutex},
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
    log::{error, warn},
};

// --

type Tx = UnboundedSender<Record>;
struct _Adapter {
    tx: Option<Tx>,
    push_id: usize,
}

pub struct Adapter(Arc<Mutex<_Adapter>>);
impl Adapter {
    pub fn new() -> Self {
        let adapter = _Adapter {
            tx: None,
            push_id: 0,
        };
        Self(Arc::new(Mutex::new(adapter)))
    }
    async fn received(&mut self, record: Record) -> ServantResult<()> {
        match record {
            Record::Report { id, msg } => {
                dbg!((&id, &msg));
                self.push(msg)
                    .await
                    .unwrap_or_else(|e| error!("{}", e.to_string()));
            }
            Record::Invoke { id, oid, req } => {
                dbg!((&id, &oid, &req));
                let ret = if let Some(oid) = &oid {
                    if let Some(servant) = ServantRegister::instance().find(oid) {
                        servant.serve(req)
                    } else {
                        Err(format!("can't find oid {} in register.", oid).into())
                    }
                } else {
                    if let Some(root) = ServantRegister::instance().root_servant() {
                        root.serve(req)
                    } else {
                        Err(format!("can't find root servant.").into())
                    }
                };
                match ret {
                    Ok(ret) => self
                        .returned(id, oid, ret)
                        .await
                        .unwrap_or_else(|e| error!("{}", e.to_string())),
                    Err(e) => warn!("{}", e.to_string()),
                }
            }
            Record::Return { .. } => unreachable!(),
        }
        Ok(())
    }
    async fn returned(&mut self, id: usize, oid: Option<Oid>, ret: Vec<u8>) -> ServantResult<()> {
        let g = self.0.lock().await;
        if let Some(mut tx) = g.tx.as_ref() {
            let record = Record::Return { id, oid, ret };
            if let Err(e) = tx.send(record).await {
                Err(e.to_string().into())
            } else {
                Ok(())
            }
        } else {
            Err("sender is none.".into())
        }
    }
    pub async fn push(&mut self, msg: PushMessage) -> ServantResult<()> {
        let mut g = self.0.lock().await;
        g.push_id += 1;
        if let Some(mut tx) = g.tx.as_ref() {
            let record = Record::Report { id: g.push_id, msg };
            if let Err(e) = tx.send(record).await {
                Err(e.to_string().into())
            } else {
                Ok(())
            }
        } else {
            Err("sender is none.".into())
        }
    }
    pub async fn run(mut self, stream: TcpStream) -> std::io::Result<()> {
        enum SelectedValue {
            None,
            Read(Bytes),
            Write(Record),
        };

        let (reader, writer) = &mut (&stream, &stream);
        let read_framed = FramedRead::new(reader, LengthCodec::<u32>::default());
        let mut write_framed = FramedWrite::new(writer, LengthCodec::<u32>::default());
        let (tx, mut rx) = unbounded();
        {
            let mut g = self.0.lock().await;
            g.tx.replace(tx);
        }

        pin_mut!(read_framed);
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
                    write_framed.send(Bytes::copy_from_slice(&v)).await?;
                }
                SelectedValue::None => break,
            }
        }

        Ok(())
    }
}
