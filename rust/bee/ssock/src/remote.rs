// -- local.rs --

use bytes::{buf::IntoBuf, Buf, BufMut, Bytes, BytesMut};
use futures::stream::Stream;
use log::{error, info, trace};
use std::{
    collections::HashMap,
    // convert::*,
    net::{SocketAddr, ToSocketAddrs},
    sync::Mutex,
};
use tokio::{
    net::{TcpListener, TcpStream},
    prelude::*,
    sync::mpsc::{channel, Sender},
};
use tokio_codec::Decoder;

use super::{
    aead::AeadCodec as CodecWithRemote,
    codec::{extract, pack, CodecError, NewDrop},
};

// --

lazy_static! {
    static ref CONN_IN_MAP: Mutex<HashMap<SocketAddr, TxInMap>> = { Mutex::new(HashMap::new()) };
    // static ref DNS_COUNTER: std::sync::atomic::AtomicU64 = std::sync::atomic::AtomicU64::new(0);
    // static ref PROXY_COUNTER: std::sync::atomic::AtomicU64 = std::sync::atomic::AtomicU64::new(0);
}
/*
struct DnsCounter;

impl DnsCounter {
    fn new() -> Self {
        println!(
            "Dns counter: {}",
            DNS_COUNTER.fetch_add(1, std::sync::atomic::Ordering::SeqCst)
        );
        DnsCounter
    }
}

impl Drop for DnsCounter {
    fn drop(&mut self) {
        println!(
            "Dns counter: {}",
            DNS_COUNTER.fetch_sub(1, std::sync::atomic::Ordering::SeqCst)
        );
    }
}

struct ProxyCounter;

impl ProxyCounter {
    fn new() -> Self {
        println!(
            "Proxy counter: {}",
            PROXY_COUNTER.fetch_add(1, std::sync::atomic::Ordering::SeqCst)
        );
        ProxyCounter
    }
}

impl Drop for ProxyCounter {
    fn drop(&mut self) {
        println!(
            "Proxy counter: {}",
            PROXY_COUNTER.fetch_sub(1, std::sync::atomic::Ordering::SeqCst)
        );
    }
}
*/
struct TxInMap {
    tx_to_permeator: Option<Sender<Bytes>>,
    tx_map: HashMap<u64, Sender<Bytes>>,
}

impl TxInMap {
    fn new() -> Self {
        Self {
            tx_to_permeator: None,
            tx_map: HashMap::new(),
        }
    }

    fn set_tx(&mut self, tx: Sender<Bytes>) {
        self.tx_to_permeator = Some(tx);
    }

    fn tx(&self) -> Sender<Bytes> {
        self.tx_to_permeator.as_ref().unwrap().clone()
    }

    fn insert_tx(&mut self, id: u64, tx: Sender<Bytes>) {
        self.tx_map.insert(id, tx);
        trace!("insert tx key at index: {}.", id);
    }
}

// --

pub fn run(addr: std::net::SocketAddr) -> Result<(), Box<dyn std::error::Error>> {
    let socket = TcpListener::bind(&addr)?;
    info!("S5_remote listening on: {}", &addr);

    let srv = socket
        .incoming()
        .map_err(|e| {
            error!("failed to accept socket; error = {:?}", e);
            e
        })
        .for_each(move |stream| {
            let inner_addr = stream.peer_addr()?;
            info!("New Connection from: {}", inner_addr);

            let (tx, rx) = channel::<Bytes>(10);
            let mut tx_in_map = TxInMap::new();
            tx_in_map.set_tx(tx);
            CONN_IN_MAP.lock().unwrap().insert(inner_addr, tx_in_map);

            let codec = CodecWithRemote::new();
            let (wt, rd) = codec.framed(stream).split();
            let h1 = rd
                .map(extract)
                .for_each(move |(id, v)| {
                    if let Some(m) = CONN_IN_MAP.lock().unwrap().get(&inner_addr) {
                        if let Some(tx1) = m.tx_map.get(&id) {
                            tokio::spawn(tx1.clone().send(v).then(|_| Ok(())));
                        } else {
                            if let Some(addr) = get_address(v, inner_addr, id) {
                                proxy_spawn(inner_addr, id, addr);
                            }
                            //  else {
                            // let rep = reply_unreachable().freeze();
                            // tokio::spawn(m.tx().send(pack(id, rep.to_vec())).then(|_|Ok(())));
                            // }
                        }
                    }

                    Ok(())
                })
                .and_then(|_| Ok(()));

            let h2 = rx.map_err(|_| CodecError).forward(wt).and_then(|_| Ok(()));

            let h = h1
                .select(h2)
                .then(move |_| {
                    info!("Local connection from {} closed.", &inner_addr);
                    CONN_IN_MAP.lock().unwrap().remove(&inner_addr);
                    // info!("conn in map: {:?}", &CONN_IN_MAP.lock().unwrap().len());
                    Ok(codec)
                })
                .and_then(|mut c| {
                    c.drop();
                    Ok(())
                });
            tokio::spawn(h);

            Ok(())
        })
        .map_err(|err| error!("error occurred: {:?}", err));

    tokio::run(srv);
    Ok(())
}

fn dns_resolve(host: &str, port: u16, inner_addr: SocketAddr, id: u64) {
    use trust_dns_resolver::config::*;
    use trust_dns_resolver::AsyncResolver;

    // Construct a new Resolver with default configuration options
    let (resolver, background) =
        AsyncResolver::new(ResolverConfig::default(), ResolverOpts::default());
    // AsyncResolver::new returns a handle for sending resolve requests and a background task
    // that must be spawned on an executor.
    tokio::spawn(background);

    // Lookup the IP addresses associated with a name.
    // This returns a future that will lookup the IP addresses, it must be run in the Core to
    //  to get the actual result.
    let lookup_future = resolver
        .lookup_ip(host)
        .and_then(move |x| {
            // 只处理解析出来的第一个ip地址
            let ip_addr = x.iter().next().unwrap();
            let addr = SocketAddr::new(ip_addr, port);
            proxy_spawn(inner_addr, id, addr);
            Ok(())
        })
        .map_err(move |_| {
            let rep = reply_unreachable().freeze();
            CONN_IN_MAP
                .lock()
                .unwrap()
                .get(&inner_addr)
                .unwrap()
                .tx()
                .send(pack(id, rep))
                .map_err(|_| ())
                .and_then(|_| Ok(()))
        })
        .then(|_| Ok(()));
    tokio::spawn(lookup_future);
}

fn get_address(value: Bytes, inner_addr: SocketAddr, id: u64) -> Option<std::net::SocketAddr> {
    use std::net::{IpAddr, Ipv4Addr, Ipv6Addr};

    // +----+-----+-------+------+----------+----------+
    // |VER | REP |  RSV  | ATYP | BND.ADDR | BND.PORT |
    // +----+-----+-------+------+----------+----------+
    // | 1  |  1  | X'00' |  1   | Variable |    2     |
    // +----+-----+-------+------+----------+----------+
    let atyp = unsafe { *value.get_unchecked(3) as usize };
    match atyp {
        1 => {
            let mut buf = value.into_buf();
            buf.advance(4);
            let a1 = buf.get_u8();
            let a2 = buf.get_u8();
            let a3 = buf.get_u8();
            let a4 = buf.get_u8();
            let port = buf.get_u16_be();
            Some(SocketAddr::new(
                IpAddr::V4(Ipv4Addr::new(a1, a2, a3, a4)),
                port,
            ))
        }
        3 => {
            let len = unsafe { *value.get_unchecked(4) as usize };
            let name = value.clone().split_off(5).split_to(len);
            let mut name = unsafe { String::from_utf8_unchecked(name.to_vec()) };
            let mut buf = value.into_buf();
            buf.advance(5 + len);
            let port = buf.get_u16_be();

            if false {
                dns_resolve(&name, port, inner_addr, id);
                None
            } else {
                name.push_str(&format!(":{}", port));
                if let Ok(mut addrs_iter) = name.to_socket_addrs() {
                    addrs_iter.next()
                } else {
                    None
                }
            }
        }
        4 => {
            let mut buf = value.into_buf();
            buf.advance(4);
            let mut v = Vec::<u16>::new();
            for _ in 0..8 {
                v.push(buf.get_u16_be());
            }
            let port = buf.get_u16_be();

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
            assert!(false);
            None
        }
    }
}

fn proxy_spawn(inner_addr: SocketAddr, id: u64, addr: SocketAddr) {
    let conn = TcpStream::connect(&addr)
        .and_then(move |stream| {
            let local_addr = stream.local_addr().unwrap();
            let addr = stream.peer_addr().unwrap();
            info!("created stream to: {}", &addr);

            let codec = super::codec::BytesCodec::new();
            let (w2, r2) = codec.framed(stream).split();

            let (mut tx, rx) = channel::<Bytes>(2);
            if let Some(m) = CONN_IN_MAP.lock().unwrap().get_mut(&inner_addr) {
                m.insert_tx(id, tx);
                tx = m.tx();
            }

            // 连接到servers成功后，要返回给local Replies信息。参见rfc-1928
            let rep = reply(local_addr);
            let rep = stream::once::<Bytes, CodecError>(Ok(rep));
            // 从server收到的包，转发给local。
            let h1 = rep
                .chain(r2)
                .map(move |v| pack(id, v))
                .forward(tx)
                .map_err(|_| println!("h1"))
                .and_then(|_| Ok(()));

            // 从local收到的包，转发给server
            let h2 = rx
                .map_err(|_| CodecError)
                .forward(w2)
                .map_err(|_| println!("h2"))
                .and_then(|_| Ok(()));

            h2.select(h1).then(move |_| {
                info!("stream to {} closed.", &addr);
                Ok(())
            })
        })
        .map_err(|err| {
            error!("connection error = {:?}", err);
        });
    tokio::spawn(conn);
}

fn reply(addr: std::net::SocketAddr) -> Bytes {
    // use std::net::{IpAddr, Ipv4Addr, Ipv6Addr, SocketAddr};

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

fn reply_unreachable() -> BytesMut {
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
        buf.put_u16_be(0)
    }
    buf
}
