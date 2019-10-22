// -- local.rs --

use bytes::Bytes;
use futures::stream::Stream;
use log::{error, info, trace, warn};
use std::{collections::HashMap, net::SocketAddr, sync::Mutex};
use tokio::{
    net::{TcpListener, TcpStream},
    prelude::*,
    sync::{
        self,
        mpsc::{channel, Sender},
    },
};
use tokio_codec::Decoder;

use super::{
    aead::AeadCodec as CodecWithRemote,
    codec::{extract, pack, CodecError, NewDrop, S5Codec}
};

// --

lazy_static! {
    static ref TX_IN_MUTEX: Mutex<TxInMap> = { Mutex::new(TxInMap::new()) };
}

struct TxInMap {
    current: u64,
    tx_to_listenor: Option<sync::oneshot::Sender<()>>,
    tx_to_permeator: Option<Sender<Bytes>>,
    tx_map: HashMap<u64, Sender<Bytes>>,
}

impl TxInMap {
    fn new() -> Self {
        Self {
            current: 0,
            tx_to_listenor: None,
            tx_to_permeator: None,
            tx_map: HashMap::new(),
        }
    }

    fn insert_tx(&mut self, rx: Sender<Bytes>) -> u64 {
        self.current += 1;
        self.tx_map.insert(self.current, rx);
        trace!("insert tx key at index: {}.", self.current);
        self.current
    }

    fn remove_tx_entry(&mut self, index: u64) {
        trace!("remove tx key {}.", index);
        if let None = self.tx_map.remove_entry(&index) {
            warn!("tx key {} has been removed already.", index);
        }
    }
}

// --

fn communicate_with_remote(
    stream: TcpStream,
    mut codec: CodecWithRemote,
) -> Result<(), std::io::Error> {
    let (w, r) = codec.framed(stream).split();
    let (tx, rx) = channel::<Bytes>(10);
    {
        TX_IN_MUTEX.lock().unwrap().tx_to_permeator = Some(tx);
    }
    let h1 = r
        .map(extract)
        .for_each(move |(id, msg)| {
            let tx_in_map = TX_IN_MUTEX.lock().unwrap();
            if let Some(t) = tx_in_map.tx_map.get(&id) {
                t.clone().send(msg).wait().unwrap();
            }

            Ok(())
        })
        .and_then(|_| Ok(()));

    let h2 = rx.map_err(|_| CodecError).forward(w).and_then(|_| Ok(()));

    let h = h1.select(h2).then(move |_| {
        info!("Connection to remote server closed.");
        // 与remote的连接中断，清除所有与client的连接，程序退出
        let mut m = TX_IN_MUTEX.lock().unwrap();
        m.tx_to_listenor = None;
        m.tx_to_permeator = None;
        m.tx_map.clear();

        codec.drop();

        Ok(())
    });
    tokio::spawn(h);

    Ok(())
}

pub fn run(
    local_addr: SocketAddr,
    remote_addr: SocketAddr,
) -> Result<(), Box<dyn std::error::Error>> {
    let srv = TcpStream::connect(&remote_addr)
        .and_then(move |s| communicate_with_remote(s, CodecWithRemote::new()))
        .map_err(|e| error!("can't connect to remote server: {:?}", e))
        .and_then(move |_| {
            if let Ok(socket) = TcpListener::bind(&local_addr) {
                info!("S5_local listening on: {}", &local_addr);
                let (tx_listenor, rx_listenor) = sync::oneshot::channel::<()>();
                TX_IN_MUTEX.lock().unwrap().tx_to_listenor = Some(tx_listenor);
                Ok((socket, rx_listenor))
            } else {
                error!("can't bind at address: {}", &local_addr);
                // 关闭与remote的连接，程序退出
                TX_IN_MUTEX.lock().unwrap().tx_to_permeator = None;
                Err(())
            }
        })
        .and_then(move |(socket, rx_listenor)| {
            socket
                .incoming()
                .map_err(|e| {
                    error!("failed to accept socket; error = {:?}", e);
                    ()
                })
                .for_each(move |stream| {
                    let client_addr = stream.peer_addr().unwrap();
                    info!("New Connection from: {}", client_addr);

                    let codec = S5Codec::new();
                    let (w, r) = codec.framed(stream).split();
                    let handle_conn = r
                        .into_future()
                        .and_then(|(_, r)| {
                            let rep = Bytes::from_static(&[5, 0]);
                            w.send(rep).then(move |w| Ok((w, r)))
                        })
                        .then(|x| {
                            if let Ok((w, r)) = x {
                                if let Ok(w) = w {
                                    return Ok((w, r));
                                }
                            }
                            error!("send to local error.");
                            Err(())
                        })
                        .and_then(|(w, r)| {
                            let (tx, rx) = channel::<Bytes>(2);
                            let (id, sender) = {
                                let mut m = TX_IN_MUTEX.lock().unwrap();
                                (m.insert_tx(tx), m.tx_to_permeator.as_ref().unwrap().clone())
                            };

                            let h1 = r
                                .map(move |x| pack(id, x))
                                .forward(sender)
                                .and_then(|_| Ok(()));
                            let h2 = rx.map_err(|_| CodecError).forward(w).and_then(|_| Ok(()));

                            h1.select(h2).then(move |_| Ok(id))
                        })
                        .map_err(|err| error!("error occurred: {:?}", err))
                        .and_then(move |id| {
                            // 与client的连接中断，删除(id, tx)
                            if let Ok(mut m) = TX_IN_MUTEX.lock() {
                                m.remove_tx_entry(id);
                            } else {
                                assert!(false);
                            }
                            info!("Connection from client closed: {}", client_addr);
                            Ok(())
                        });
                    tokio::spawn(handle_conn);

                    Ok(())
                })
                .select(rx_listenor.map_err(|_| ()))
                .then(|_| {
                    info!("Listen socket quit.");
                    Ok(())
                })
        })
        .and_then(|_| {
            info!("tokio::run() end.");
            Ok(())
        });
    tokio::run(srv);
    Ok(())
}
