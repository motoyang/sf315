// -- remote.rs --

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
    bytes::{Buf, BufMut, Bytes, BytesMut},
    codec::LengthCodec,
    futures::{
        channel::mpsc::{channel, unbounded, Receiver, Sender, UnboundedSender},
        // future::join_all,
        pin_mut, select,
        sink::SinkExt,
        FutureExt as _,
    },
    futures_codec::{BytesCodec, FramedRead, FramedWrite},
    log::{info, trace, warn},
    std::collections::HashMap,
};

// --

#[derive(Clone, Debug)]
struct ClientsMap(Arc<Mutex<HashMap<u64, Sender<Bytes>>>>);

impl ClientsMap {
    fn new() -> Self {
        Self(Arc::new(Mutex::new(HashMap::new())))
    }
    async fn insert(&mut self, id: u64, tx: Sender<Bytes>) {
        self.0.lock().await.insert(id, tx);
    }
    async fn remove(&mut self, id: u64) {
        self.0.lock().await.remove(&id);
    }
    async fn get<'a>(&'a mut self, id: u64) -> Option<Sender<Bytes>> {
        if let Some(v) = self.0.lock().await.get(&id) {
            Some(v.clone())
        } else {
            None
        }
    }
}

// --

pub fn run(remote_addr: SocketAddr) -> BoxResult<()> {
    let r = task::block_on(accept_on(remote_addr));
    // let accept_handle = task::spawn(accept_on(remote_addr));
    // let v = vec![accept_handle];
    // let r = task::block_on(join_all(v));
    info!("run result: {:?}", r);

    Ok(())
}

async fn accept_on(addr: impl ToSocketAddrs) -> BoxResult<()> {
    let listener = TcpListener::bind(addr).await?;
    let mut incoming = listener.incoming();
    while let Some(stream) = incoming.next().await {
        let stream = stream?;
        info!("Accepting from: {}", stream.peer_addr()?);
        let _handle = task::spawn(connection_loop(stream));
    }
    Ok(())
}

async fn connection_loop(stream: TcpStream) -> BoxResult<()> {
    enum SelectedValue {
        None,
        Read(Bytes),
        Write(Bytes),
    };

    let (reader, writer) = &mut (&stream, &stream);
    let read_framed = FramedRead::new(reader, LengthCodec::<u32>::default());
    let mut write_framed = FramedWrite::new(writer, LengthCodec::<u32>::default());
    let mut clients_map = ClientsMap::new();
    let (tx_to_local, mut rx) = unbounded();

    pin_mut!(read_framed);
    loop {
        let value = select! {
            from_local = read_framed.next().fuse() => match from_local {
                Some(msg) => SelectedValue::Read(msg?),
                None => SelectedValue::None,
            },
            to_local = rx.next().fuse() => match to_local {
                Some(msg) => SelectedValue::Write(msg),
                None => SelectedValue::None,
            },
        };
        match value {
            SelectedValue::Read(msg) => {
                let (id, msg) = extract(msg);
                if let Some(mut tx) = clients_map.get(id).await {
                    tx.send(msg).await?;
                } else {
                    if let Some(addr) = get_address(msg.clone()).await {
                        let (tx_to_server, rx_from_server) = channel::<Bytes>(1);
                        clients_map.insert(id, tx_to_server).await;
                        task::spawn(connect_to(
                            addr,
                            tx_to_local.clone(),
                            rx_from_server,
                            id,
                            clients_map.clone(),
                        ));
                    } else {
                        let rep = reply_unreachable();
                        write_framed.send(rep).await?;
                        warn!("reply_unreachable. id = {}, msg = {:?}", id, msg);
                    }
                }
            }
            SelectedValue::Write(msg) => {
                write_framed.send(msg).await?;
            }
            SelectedValue::None => break,
        }
    }

    Ok(())
}

async fn connect_to(
    server_addr: SocketAddr,
    mut tx_to_local: UnboundedSender<Bytes>,
    mut rx: Receiver<Bytes>,
    id: u64,
    clients: ClientsMap,
) -> BoxResult<()> {
    enum SelectedValue {
        None,
        Read(Bytes),
        Write(Bytes),
    };

    let _connect_clean = DropGuard::new((id, clients.clone()), |(x, mut c)| {
        task::block_on(async move {
            trace!("connect clean: id = {}", id);
            c.remove(x).await;
        });
    });

    let stream = TcpStream::connect(server_addr).await?;
    let (reader, writer) = (&stream, &stream);
    let read_framed = FramedRead::new(reader, BytesCodec {});
    let mut write_framed = FramedWrite::new(writer, BytesCodec {});

    // 连接到servers成功后，要返回给local Replies信息。参见rfc-1928
    let rep = reply(stream.local_addr()?);
    tx_to_local.send(pack(id, rep)).await?;

    pin_mut!(read_framed);
    loop {
        let value = select! {
            msg_from = read_framed.next().fuse() => match msg_from {
                Some(msg) => SelectedValue::Read(msg?),
                None => SelectedValue::None,
            },

            msg_to = rx.next().fuse() => match msg_to {
                Some(msg) => SelectedValue::Write(msg),
                None => SelectedValue::None,
            },
        };

        match value {
            SelectedValue::Read(msg) => {
                tx_to_local.send(pack(id, msg)).await?;
            }
            SelectedValue::Write(msg) => {
                write_framed.send(msg).await?;
            }
            SelectedValue::None => {
                break;
            }
        }
    }
    Ok(())
}

async fn get_address(value: Bytes) -> Option<std::net::SocketAddr> {
    use std::net::{IpAddr, Ipv4Addr, Ipv6Addr};

    // +----+-----+-------+------+----------+----------+
    // |VER | REP |  RSV  | ATYP | BND.ADDR | BND.PORT |
    // +----+-----+-------+------+----------+----------+
    // | 1  |  1  | X'00' |  1   | Variable |    2     |
    // +----+-----+-------+------+----------+----------+
    let atyp = unsafe { *value.get_unchecked(3) as usize };
    match atyp {
        1 => {
            let mut buf = value;
            buf.advance(4);
            let a1 = buf.get_u8();
            let a2 = buf.get_u8();
            let a3 = buf.get_u8();
            let a4 = buf.get_u8();
            let port = buf.get_u16();
            Some(SocketAddr::new(
                IpAddr::V4(Ipv4Addr::new(a1, a2, a3, a4)),
                port,
            ))
        }
        3 => {
            let len = unsafe { *value.get_unchecked(4) as usize };
            let name = value.clone().split_off(5).split_to(len);
            let mut name = unsafe { String::from_utf8_unchecked(name.to_vec()) };
            let mut buf = value;
            buf.advance(5 + len);
            let port = buf.get_u16();

            name.push_str(&format!(":{}", port));
            if let Ok(mut addrs_iter) = name.to_socket_addrs().await {
                addrs_iter.next()
            } else {
                None
            }
        }
        4 => {
            let mut buf = value;
            buf.advance(4);
            let mut v = Vec::<u16>::new();
            for _ in 0..8 {
                v.push(buf.get_u16());
            }
            let port = buf.get_u16();

            let ipv6 = unsafe {
                Ipv6Addr::new(
                    *v.get_unchecked(0),
                    *v.get_unchecked(1),
                    *v.get_unchecked(2),
                    *v.get_unchecked(3),
                    *v.get_unchecked(4),
                    *v.get_unchecked(5),
                    *v.get_unchecked(6),
                    *v.get_unchecked(7),
                )
            };
            Some(SocketAddr::new(IpAddr::V6(ipv6), port))
        }
        _ => {
            // assert!(false);
            None
        }
    }
}

fn reply(addr: std::net::SocketAddr) -> Bytes {
    // +----+-----+-------+------+----------+----------+
    // |VER | REP |  RSV  | ATYP | BND.ADDR | BND.PORT |
    // +----+-----+-------+------+----------+----------+
    // | 1  |  1  | X'00' |  1   | Variable |    2     |
    // +----+-----+-------+------+----------+----------+
    let mut buf = BytesMut::with_capacity(6 + 16);
    buf.put_u8(5);
    buf.put_u8(0);
    buf.put_u8(0);
    match addr {
        SocketAddr::V4(ipv4) => {
            buf.put_u8(1);
            let v: Vec<u8> = ipv4.ip().octets().iter().rev().map(|&x| x).collect();
            buf.put_slice(&v);
        }
        SocketAddr::V6(ipv6) => {
            buf.put_u8(4);
            buf.put_slice(&ipv6.ip().octets());
        }
    }
    // buf.put_u16_be(addr.port());
    buf.put_u16_le(addr.port());
    buf.freeze()
}

fn reply_unreachable() -> Bytes {
    // +----+-----+-------+------+----------+----------+
    // |VER | REP |  RSV  | ATYP | BND.ADDR | BND.PORT |
    // +----+-----+-------+------+----------+----------+
    // | 1  |  1  | X'00' |  1   | Variable |    2     |
    // +----+-----+-------+------+----------+----------+
    let mut buf = BytesMut::with_capacity(6 + 4);
    buf.put_u8(5);
    buf.put_u8(4); // REP: Host unreachable
    buf.put_u8(0); // RSV: 0
    buf.put_u8(1); // ATYP: ipv4
    for _ in 0..3 {
        buf.put_u16(0)
    }
    buf.freeze()
}
