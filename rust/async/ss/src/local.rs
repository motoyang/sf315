// -- local.rs --
use {
    crate::{
        common::{extract, pack, BoxResult},
        drop_guard::DropGuard,
    },
    async_std::{
        net::{SocketAddr, TcpListener, TcpStream, ToSocketAddrs},
        prelude::*,
        // prelude::FutureExt as _,
        // stream,
        sync::{channel, Arc, Mutex, Sender},
        task,
    },
    bytes::{BufMut, Bytes, BytesMut},
    codec::LengthCodec,
    futures::{future::join_all, pin_mut, select, sink::SinkExt, FutureExt as _},
    futures_codec::{Decoder, Encoder, FramedRead, FramedWrite},
    std::{collections::HashMap, time::Duration},
};

// --

pub fn run(local_addr: SocketAddr, remote_addr: SocketAddr) -> BoxResult<()> {
    // println!("l: {:?}, r: {:?}", local_addr, remote_addr);
    let d = DispatcherWraper::new();
    let remote_handle = task::spawn(connect_to(remote_addr, d.clone()));
    let accept_handle = task::spawn(accept_on(local_addr, d.clone()));
    let v = vec![remote_handle, accept_handle];
    let r = task::block_on(join_all(v));
    println!("run result: {:?}", r);
    println!("dispatcher: {:?}", d);

    Ok(())
}

async fn connect_to(addr: impl ToSocketAddrs, mut dispatcher: DispatcherWraper) -> BoxResult<()> {
    enum SelectedValue {
        None,
        Read(Bytes),
        Write(Bytes),
        // Interval(Bytes),
    };

    let stream = TcpStream::connect(addr).await?;
    let (reader, writer) = (&stream, &stream);
    let read_framed = FramedRead::new(reader, LengthCodec::<u32>::new());
    let mut write_framed = FramedWrite::new(writer, LengthCodec::<u32>::new());

    let (tx, rx) = channel(1);
    dispatcher.set_remote_tx(Some(tx)).await;
    let _dispatcher_clean = DropGuard::new(dispatcher.clone(), |mut d| {
        task::block_on(async move {
            d.clean().await;
        });
    });

    // let mut i = 0_usize;
    // let interval = stream::interval(Duration::from_secs(30));
    pin_mut!(read_framed, rx);
    loop {
        let value = select! {
            msg_to_client = read_framed.next().fuse() => match msg_to_client {
                Some(msg) => SelectedValue::Read(msg?),
                None => SelectedValue::None,
            },

            msg_from_client = rx.next().fuse() => match msg_from_client {
                Some(msg) => SelectedValue::Write(msg),
                None => SelectedValue::None,
            },

            // hello = interval.next().fuse() => match hello {
            //     Some(hello) if i < 6666 => {
            //         i += 1;
            //         SelectedValue::Interval(Bytes::from(format!("Hello World! #{}", i)))
            //     },
            //     _ => SelectedValue::None,
            // }
        };

        match value {
            SelectedValue::Read(msg) => {
                println!("{:?}", msg);
                let (id, msg) = extract(msg);
                if let Some(c) = dispatcher.find_client(id).await {
                    c.send(msg).await;
                }
            },
            SelectedValue::Write(msg) => {
                write_framed.send(msg).await?;
            }
            // SelectedValue::Interval(msg) => {
            //     write_framed.send(msg).await?;
            // }
            SelectedValue::None => {
                break;
            }
        }
    }
    Ok(())
}

async fn accept_on(addr: impl ToSocketAddrs, mut dispatcher: DispatcherWraper) -> BoxResult<()> {
    enum SelectedValue {
        None,
        Client(TcpStream),
    }

    // 等待remote_sender被connecto_to函数设置为!none
    while dispatcher.remote_tx_is_none().await {
        task::sleep(Duration::from_millis(10)).await;
    }

    let _dispatcher_clean = DropGuard::new(dispatcher.clone(), |mut d| {
        task::block_on(async move {
            d.clean().await;
        });
    });

    let listener = TcpListener::bind(addr).await?;
    let (tx, rx) = channel::<u8>(1);
    dispatcher.set_accept_tx(Some(tx)).await;

    let incoming = listener.incoming();
    pin_mut!(incoming, rx);
    loop {
        let value = select! {
            client_stream = incoming.next().fuse() => match client_stream {
                Some(stream) => SelectedValue::Client(stream?),
                None => SelectedValue::None,
            },
            close_req = rx.next().fuse() => SelectedValue::None,
        };
        match value {
            SelectedValue::Client(stream) => {
                println!("Accepting from: {}", stream.peer_addr()?);
                task::spawn(connection_loop(dispatcher.clone(), stream));
            }
            _ => break,
        }
    }
    Ok(())
}

async fn connection_loop(mut dispatcher: DispatcherWraper, stream: TcpStream) -> BoxResult<()> {
    enum SelectedValue {
        None,
        Read(Bytes),
        Write(Bytes),
    }

    let (tx, rx) = channel(3);
    let (tx, id) =
        (dispatcher.remote_tx().await, dispatcher.insert_client(tx).await);
    let _remove_client = DropGuard::new((id, dispatcher.clone()), |(x, mut d)| {
        task::block_on(async move {
            d.remove_client(x).await;
        });
    });

    let (reader, writer) = (&stream, &stream);
    let read_framed = FramedRead::new(reader, S5Codec::new());
    let mut write_framed = FramedWrite::new(writer, S5Codec::new());

    let mut req = 0_usize;
    pin_mut!(read_framed, rx);
    loop {
        let value = select! {
        msg_from_client = read_framed.next().fuse() => match msg_from_client {
                Some(msg) if msg.is_ok() => SelectedValue::Read(msg?),
                _ => SelectedValue::None,
            },
        msg_to_client = rx.next().fuse() => match msg_to_client {
            Some(msg) => SelectedValue::Write(msg),
            None => SelectedValue::None,
        }
        };
        match value {
            SelectedValue::Read(msg) => {
                if req == 0 {
                    let rep = Bytes::from_static(&[5, 0]);
                    write_framed.send(rep).await?;
                } else {
                    tx.send(pack(id, msg)).await;
                }
                req += 1;
            }
            SelectedValue::Write(msg) => {
                write_framed.send(msg).await?
            }
            SelectedValue::None => break,
        }
    }

    Ok(())
}

// --

#[derive(Debug, Clone)]
struct Dispatcher {
    index_of_client: u64,
    remote_sender: Option<Sender<Bytes>>,
    accept_sender: Option<Sender<u8>>,
    client_senders: HashMap<u64, Sender<Bytes>>,
}

#[derive(Debug, Clone)]
struct DispatcherWraper(Arc<Mutex<Dispatcher>>);

impl DispatcherWraper {
    pub fn new() -> Self {
        Self (Arc::new(Mutex::new(Dispatcher {
            index_of_client: 0,
            remote_sender: None,
            accept_sender: None,
            client_senders: HashMap::new(),
        })))
    }

    async fn clean(&mut self) {
        let mut g = self.0.lock().await;
        g.client_senders.clear();
        g.remote_sender.take();
        g.accept_sender.take();
    }

    async fn insert_client(&mut self, tx: Sender<Bytes>) -> u64 {
        let mut g = self.0.lock().await;
        g.index_of_client += 1;
        let id = g.index_of_client;
        g.client_senders.insert(id, tx);
        id
    }
    async fn remove_client(&mut self, index: u64) {
        let mut g = self.0.lock().await;
        g.client_senders.remove(&index);
    }
    async fn find_client(&self, index: u64) -> Option<Sender<Bytes>> {
        let g = self.0.lock().await;
        if let Some(s) = g.client_senders.get(&index) {
            Some(s.clone())
        } else {
            None
        }
    }
    async fn set_remote_tx(&mut self, tx: Option<Sender<Bytes>>) {
        let mut g = self.0.lock().await;
        g.remote_sender = tx;
    }
    async fn remote_tx_is_none(&self) -> bool {
        let g = self.0.lock().await;
        g.remote_sender.is_none()
    }
    async fn remote_tx(&self) -> Sender<Bytes> {
        let g = self.0.lock().await;
        assert!(g.remote_sender.is_some());
        g.remote_sender.as_ref().unwrap().clone()
    }
    async fn set_accept_tx(&mut self, tx: Option<Sender<u8>>) {
        let mut g = self.0.lock().await;
        g.accept_sender = tx;
    }
    // async fn accept_tx(&self) -> Sender<u8> {
    //     let g = self.0.lock().await;
    //     assert!(g.accept_sender.is_some());
    //     g.accept_sender.as_ref().unwrap().clone()
    // }
}

// --

#[derive(Copy, Clone, Debug)]
struct S5Codec(usize);

impl S5Codec {
    fn new() -> Self {
        Self(0)
    }
}

impl Encoder for S5Codec {
    type Item = Bytes;
    type Error = std::io::Error;

    fn encode(&mut self, line: Self::Item, buf: &mut BytesMut) -> Result<(), Self::Error> {
        let body_len = line.len();
        if body_len == 0 {
            return Ok(());
        }

        if self.0 == 1 && 0 == unsafe { *line.get_unchecked(1) } {
            self.0 += 1;
        }

        buf.reserve(body_len);
        buf.put(line);
        Ok(())
    }
}

impl Decoder for S5Codec {
    type Item = Bytes;
    type Error = std::io::Error;

    fn decode(&mut self, buf: &mut BytesMut) -> Result<Option<Self::Item>, Self::Error> {
        if buf.len() < 3 {
            return Ok(None);
        }

        println!("self.0 = {}, buf = {:?}", self.0, buf.to_vec());
        match self.0 {
            0 => {
                // +----+----------+----------+
                // |VER | NMETHODS | METHODS  |
                // +----+----------+----------+
                // | 1  |    1     | 1 to 255 |
                // +----+----------+----------+
                let nmethods = unsafe { *buf.get_unchecked(1) as usize };
                let len = 2 + nmethods;
                if len <= buf.len() {
                    self.0 += 2;
                    return Ok(Some(buf.split_to(len).freeze()));
                } else {
                    return Ok(None);
                }
            }
            1 => {
                // +----+------+----------+------+----------+
                // |VER | ULEN |  UNAME   | PLEN |  PASSWD  |
                // +----+------+----------+------+----------+
                // | 1  |  1   | 1 to 255 |  1   | 1 to 255 |
                // +----+------+----------+------+----------+
                let ulen = unsafe { *buf.get_unchecked(1) as usize };
                let plen = if ulen > 1 {
                    unsafe { *buf.get_unchecked(2 + ulen) as usize }
                } else {
                    0
                };
                let len = if plen == 0 { 2 + ulen } else { 3 + ulen + plen };
                if len <= buf.len() {
                    self.0 += 1;
                    return Ok(Some(buf.split_to(len).freeze()));
                } else {
                    return Ok(None);
                }
            }
            2 => {
                // +----+-----+-------+------+----------+----------+
                // |VER | CMD |  RSV  | ATYP | DST.ADDR | DST.PORT |
                // +----+-----+-------+------+----------+----------+
                // | 1  |  1  | X'00' |  1   | Variable |    2     |
                // +----+-----+-------+------+----------+----------+
                let atyp = unsafe { *buf.get_unchecked(3) as usize };
                let addr_len = match atyp {
                    1 => 4,
                    3 => unsafe { *buf.get_unchecked(4) as usize + 1 },
                    4 => 16,
                    _ => {
                        assert!(false);
                        0
                    }
                };
                let len = 4 + addr_len + 2;
                if len <= buf.len() {
                    self.0 += 1;
                    return Ok(Some(buf.split_to(len).freeze()));
                } else {
                    return Ok(None);
                }
            }
            _ => Ok(Some(buf.split_to(buf.len()).freeze())),
        }
    }
}
