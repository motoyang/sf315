// -- server.rs --

use bytes::Bytes;
use log::trace;
use std::time::{Duration, Instant};
use tokio::{prelude::*, timer::Interval};

use bee2::{app::Application, codec::AeadCodec};

// --

#[derive(Copy, Clone)]
pub struct Server;

impl Application for Server {
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
        match msg {
            Ok(x) => {
                let mut x = x.to_vec();
                x.reverse();
                Ok(Some(x))
            }
            Err(e) => Err(e),
        }
    }

    fn notify(
        &self,
    ) -> Box<dyn Stream<Item = Option<Vec<u8>>, Error = std::io::Error> + std::marker::Send> {
        // Box::new(stream::iter_ok::<_, Self::Error>(vec![vec![17], vec![19]]))
        let mut n = 0_usize;
        let interval = Interval::new(Instant::now(), Duration::from_secs(1))
            .take(8)
            .map_err(bad_utf8)
            .map(move |_| {
                n %= 26;
                n += 5;
                Some(gen_vec2(n))
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

// Turns string errors into std::io::Error
pub fn bad_utf8<E>(_: E) -> std::io::Error {
    std::io::Error::new(
        std::io::ErrorKind::InvalidData,
        "Unable to decode input as UTF8",
    )
}
