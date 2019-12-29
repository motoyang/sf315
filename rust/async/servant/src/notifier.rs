// -- notifier.rs --

use {
    super::servant::{Record, ServantResult},
    async_std::sync::{Arc, Mutex},
    futures::{
        channel::mpsc::{unbounded, UnboundedSender},
        select,
        sink::SinkExt,
        stream::StreamExt,
        FutureExt as _,
        pin_mut,
    },
    log::warn,
    std::{collections::HashMap, net::SocketAddr},
};

// --

lazy_static! {
    static ref NOTIFIER: Notifier = Notifier(Arc::new(Mutex::new(_Notifier {
        id: 0,
        tx: None,
        senders: HashMap::new(),
    })));
}

struct _Notifier {
    id: usize,
    tx: Option<UnboundedSender<Record>>,
    senders: HashMap<SocketAddr, UnboundedSender<Record>>,
}

#[derive(Clone)]
pub struct Notifier(Arc<Mutex<_Notifier>>);
impl Notifier {
    pub fn instance() -> Self {
        NOTIFIER.clone()
    }
    pub async fn clean(&self) {
        let mut g = self.0.lock().await;
        g.tx.take();
        g.senders.clear();
    }
    pub async fn insert(&self, addr: SocketAddr, tx: UnboundedSender<Record>) {
        let mut g = self.0.lock().await;
        g.senders.insert(addr, tx);
    }
    pub async fn remove(&self, addr: &SocketAddr) {
        let mut g = self.0.lock().await;
        g.senders.remove(addr);
    }
    pub async fn send(&self, msg: Vec<u8>) -> ServantResult<()> {
        let mut g = self.0.lock().await;
        g.id += 1;
        let notice = Record::Notice { id: g.id, msg };
        let mut s = g.tx.as_ref().unwrap();
        match s.send(notice).await {
            Ok(_) => Ok(()),
            Err(e) => Err(e.to_string().into()),
        }
    }
    pub async fn run(self) -> ServantResult<()> {
        enum SelectedValue {
            None,
            Write(Record),
        };

        let (tx, rx) = unbounded();
        {
            let mut g = self.0.lock().await;
            g.tx.replace(tx);
        }

        pin_mut!(rx);
        loop {
            let value = select! {
                send_msg = rx.next().fuse() => match send_msg {
                    Some(msg) => SelectedValue::Write(msg),
                    None => SelectedValue::None,
                },
            };
            match value {
                SelectedValue::Write(msg) => {
                    let g = self.0.lock().await;
                    let mut values = g.senders.values();
                    while let Some(mut s) = values.next() {
                        if let Err(e) = s.send(msg.clone()).await {
                            warn!("{}", e.to_string());
                        }
                    }
                }
                SelectedValue::None => break,
            }
        }

        Ok(())
    }
}
