// -- local.rs --

use {
    crate::{
        common::{extract, pack, BoxResult},
        drop_guard::DropGuard,
    },
    async_std::{
        net::{SocketAddr, TcpListener, TcpStream, ToSocketAddrs},
        prelude::*,
        sync::{Arc, Mutex},
        task,
    },
    bytes::{BufMut, Bytes, BytesMut},
    codec::LengthCodec,
    futures::{
        channel::mpsc::{channel, unbounded, Sender, UnboundedSender},
        future::join_all,
        pin_mut, select,
        sink::SinkExt,
        FutureExt as _,
    },
    futures_codec::{Decoder, Encoder, FramedRead, FramedWrite},
    log::{info, trace, warn},
    std::{collections::HashMap, time::Duration},
};

// --

pub fn run(local_addr: SocketAddr, remote_addr: SocketAddr) -> BoxResult<()> {
    let d = DispatcherWraper::new();
    let remote_handle = task::spawn(connect_to(remote_addr, d.clone()));
    let accept_handle = task::spawn(accept_on(local_addr, d.clone()));
    let v = vec![remote_handle, accept_handle];
    let r = task::block_on(join_all(v));
    info!("run result: {:?}", r);
    info!("dispatcher: {:?}", d);

    Ok(())
}

async fn connect_to(addr: impl ToSocketAddrs, mut dispatcher: DispatcherWraper) -> BoxResult<()> {
    #[derive(Debug)]
    enum SelectedValue {
        ReadNone,
        ReadError(std::io::Error),
        Read(Bytes),
        WriteNone,
        Write(Bytes),
    }

    let stream = match TcpStream::connect(addr).await {
        Ok(s) => s,
        Err(e) => {
            warn!("connect err: {:?}", e);
            return Err(e)?;
        }
    };
    let (reader, writer) = (&stream, &stream);
    let read_framed = FramedRead::new(reader, LengthCodec::<u32>::default());
    let mut write_framed = FramedWrite::new(writer, LengthCodec::<u32>::default());

    let (tx, mut rx) = unbounded();
    dispatcher.set_remote_tx(Some(tx)).await;
    let _dispatcher_clean = DropGuard::new(dispatcher.clone(), |mut d| {
        task::block_on(async move {
            info!("connect_to quit.");
            d.clean().await;
        });
    });

    pin_mut!(read_framed);
    loop {
        let value = select! {
            msg_to_client = read_framed.next().fuse() => match msg_to_client {
                Some(msg) => match msg {
                    Ok(msg) => SelectedValue::Read(msg),
                    Err(e) => SelectedValue::ReadError(e),
                },
                None => SelectedValue::ReadNone,
            },
            msg_from_client = rx.next().fuse() => match msg_from_client {
                Some(msg) => SelectedValue::Write(msg),
                None => SelectedValue::WriteNone,
            },
        };

        match value {
            SelectedValue::Read(msg) => {
                let (id, msg) = extract(msg);
                if let Some(mut tx) = dispatcher.find_client(id).await {
                    if let Err(e) = tx.send(msg).await {
                        warn!("tx.send() error: {:?}", e);
                        return Err(e)?;
                    }
                }
            }
            SelectedValue::Write(msg) => {
                if let Err(e) = write_framed.send(msg).await {
                    warn!("write to remote error: {:?}", e);
                    return Err(e)?;
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

async fn accept_on(addr: impl ToSocketAddrs, mut dispatcher: DispatcherWraper) -> BoxResult<()> {
    #[derive(Debug)]
    enum SelectedValue {
        CloseNone,
        ClientNone,
        ClientError(std::io::Error),
        Client(TcpStream),
    }

    // 等待remote_sender被connecto_to函数设置为!none
    while dispatcher.remote_tx_is_none().await {
        task::sleep(Duration::from_millis(10)).await;
    }

    let _dispatcher_clean = DropGuard::new(dispatcher.clone(), |mut d| {
        task::block_on(async move {
            info!("accept_on quit.");
            d.clean().await;
        });
    });

    let listener = match TcpListener::bind(addr).await {
        Ok(l) => l,
        Err(e) => {
            warn!("bind error: {:?}", e);
            return Err(e)?;
        }
    };
    let (tx, mut rx) = channel::<u8>(1);
    dispatcher.set_accept_tx(Some(tx)).await;

    let incoming = listener.incoming();
    pin_mut!(incoming);
    loop {
        let value = select! {
            client_stream = incoming.next().fuse() => match client_stream {
                Some(stream) => match stream {
                    Ok(s) => SelectedValue::Client(s),
                    Err(e) => SelectedValue::ClientError(e)
                },
                None => SelectedValue::ClientNone,
            },
            close_req = rx.next().fuse() => SelectedValue::CloseNone,
        };
        match value {
            SelectedValue::Client(stream) => {
                let p = match stream.peer_addr() {
                    Ok(p) => p,
                    Err(e) => {
                        warn!("peer_addr() error: {:?}", e);
                        return Err(e)?;
                    }
                };
                info!("Accepting from: {}", p);
                task::spawn(connection_loop(dispatcher.clone(), stream));
            }
            SelectedValue::ClientError(e) => {
                warn!("ClientError: {:?}", e);
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

async fn connection_loop(mut dispatcher: DispatcherWraper, stream: TcpStream) -> BoxResult<()> {
    #[derive(Debug)]
    enum SelectedValue {
        ReadNone,
        ReadError(std::io::Error),
        Read(Bytes),
        WriteNone,
        Write(Bytes),
    }

    let (tx, mut rx) = channel(1);
    let (mut tx, id) = (
        dispatcher.remote_tx().await,
        dispatcher.insert_client(tx).await,
    );
    let _remove_client = DropGuard::new((id, dispatcher.clone()), |(x, mut d)| {
        task::block_on(async move {
            trace!("connect clean: id = {}", id);
            d.remove_client(x).await;
        });
    });

    let (reader, writer) = (&stream, &stream);
    let read_framed = FramedRead::new(reader, S5Codec::new());
    let mut write_framed = FramedWrite::new(writer, S5Codec::new());

    let mut req = 0_usize;
    pin_mut!(read_framed);
    loop {
        let value = select! {
            msg_from_client = read_framed.next().fuse() => match msg_from_client {
                Some(msg) => match msg{
                    Ok(msg) => SelectedValue::Read(msg),
                    Err(e) => SelectedValue::ReadError(e),
                },
                None => SelectedValue::ReadNone,
            },
            msg_to_client = rx.next().fuse() => match msg_to_client {
                Some(msg) => SelectedValue::Write(msg),
                None => SelectedValue::WriteNone,
            }
        };
        match value {
            SelectedValue::Read(msg) => {
                if req == 0 {
                    let rep = Bytes::from_static(&[5, 0]);
                    if let Err(e) = write_framed.send(rep).await {
                        warn!("send error: {:?}", e);
                        return Err(e)?;
                    }
                } else {
                    if let Err(e) = tx.send(pack(id, msg)).await {
                        warn!("tx.send() error: {:?}", e);
                        return Err(e)?;
                    }
                }
                req += 1;
            }
            SelectedValue::Write(msg) => {
                if let Err(e) = write_framed.send(msg).await {
                    warn!("send error: {:?}", e);
                    return Err(e)?;
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

// --

#[derive(Debug)]
struct Dispatcher {
    index_of_client: u64,
    remote_sender: Option<UnboundedSender<Bytes>>,
    accept_sender: Option<Sender<u8>>,
    client_senders: HashMap<u64, Sender<Bytes>>,
}

#[derive(Debug, Clone)]
struct DispatcherWraper(Arc<Mutex<Dispatcher>>);

impl DispatcherWraper {
    pub fn new() -> Self {
        Self(Arc::new(Mutex::new(Dispatcher {
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
    async fn set_remote_tx(&mut self, tx: Option<UnboundedSender<Bytes>>) {
        let mut g = self.0.lock().await;
        g.remote_sender = tx;
    }
    async fn remote_tx_is_none(&self) -> bool {
        let g = self.0.lock().await;
        g.remote_sender.is_none()
    }
    async fn remote_tx(&self) -> UnboundedSender<Bytes> {
        let g = self.0.lock().await;
        assert!(g.remote_sender.is_some());
        g.remote_sender.as_ref().unwrap().clone()
    }
    async fn set_accept_tx(&mut self, tx: Option<Sender<u8>>) {
        let mut g = self.0.lock().await;
        g.accept_sender = tx;
    }
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
