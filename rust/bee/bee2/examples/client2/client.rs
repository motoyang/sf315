// -- client.rs --

use bytes::Bytes;
use log::trace;
use std::time::{Duration, Instant};
use tokio::{prelude::*, timer::Interval};

use bee2::{app::Application, codec::AeadCodec};

// --

pub struct Client;

impl Application for Client {
    type Codec = AeadCodec;

    fn addr(&self) -> std::net::SocketAddr {
        let addr = "127.0.0.1:6142".to_string();
        addr.parse().unwrap()
    }

    fn take_over(
        &self,
        msg: Result<Bytes, std::io::Error>,
    ) -> Result<Option<Vec<u8>>, std::io::Error> {
        trace!("{:?}", msg);
        if let Err(e) = msg {
            Err(e)
        } else {
            Ok(None)
        }
    }

    fn notify(
        &self,
    ) -> Box<dyn Stream<Item = Option<Vec<u8>>, Error = std::io::Error> + std::marker::Send> {
        let mut n = 0_usize;
        let mut m = 0_usize;
        let interval = Interval::new(Instant::now(), Duration::from_millis(10))
            // .take(88888)
            .then(move |v| {
                m += 1;
                if m > 88888 {
                    return Err(std::io::Error::new(
                        std::io::ErrorKind::InvalidData,
                        "end of stream.",
                    ))
                }
                if let Ok(v) = v {
                    Ok(v)
                } else {
                    Err(std::io::Error::new(
                    std::io::ErrorKind::InvalidData,
                    "Interval error",))
                }
            })
            .then(move |v| match v {
                Ok(_) => {
                    n %= 126;
                    n += 1;
                    let v = gen_vec2(n);
                    Ok(Some(v))
                }
                _ => Err(std::io::Error::new(
                    std::io::ErrorKind::InvalidData,
                    "Interval error in notify function",
                )),
            });
        Box::new(interval)
    }
}

fn gen_vec2(c: usize) -> Vec<u8> {
    let mut v = Vec::with_capacity(c);
    for i in 0..c {
        v.push(i as u8);
    }
    v
}
/*
// Turns string errors into std::io::Error
pub fn bad_utf8<E>(_: E) -> std::io::Error {
    std::io::Error::new(
        std::io::ErrorKind::InvalidData,
        "Unable to decode input as UTF8",
    )
}
*/
