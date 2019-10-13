// Turns string errors into std::io::Error
pub fn bad_utf8<E>(_: E) -> std::io::Error {
    std::io::Error::new(
        std::io::ErrorKind::InvalidData,
        "Unable to decode input as UTF8",
    )
}

// --

use bytes::{Buf, BufMut, BytesMut, IntoBuf};
use futures::stream::{Stream};
use tokio::{prelude::*, net::TcpStream, timer::Interval};
// use tokio::;
// use tokio::timer::Interval;
use tokio_codec::{Decoder, Encoder};
use std::time::{Duration, Instant};

pub struct RecordCodec {
    head: u16,
}

impl RecordCodec {
    pub fn new() -> RecordCodec {
        RecordCodec { head: 0 }
    }
}

impl Encoder for RecordCodec {
    type Item = Vec<u8>;
    type Error = std::io::Error;

    fn encode(&mut self, line: Self::Item, buf: &mut BytesMut) -> Result<(), Self::Error> {
        buf.reserve(line.len() + std::mem::size_of_val(&self.head));
        buf.put_u16_le(line.len() as u16);
        buf.put(line);
        Ok(())
    }
}

impl Decoder for RecordCodec {
    type Item = Vec<u8>;
    type Error = std::io::Error;

    // Find the next line in buf!
    fn decode(&mut self, buf: &mut BytesMut) -> Result<Option<Self::Item>, Self::Error> {
        let head_len = std::mem::size_of_val(&self.head);
        let buf_len = buf.len();
        let mut b = buf.clone().freeze().into_buf();
        let len = if buf_len > head_len {
            b.get_u16_le() as usize
        } else {
            return Ok(None);
        };

        Ok(if len + head_len <= buf_len {
            let b = buf.split_to(head_len + len).split_off(head_len);
            Some(b.to_vec())
        } else {
            None
        })
    }
    /*
      // Find the next line in buf when there will be no more data coming.
      fn decode_eof(&mut self, buf: &mut BytesMut) -> Result<Option<Self::Item>, Self::Error> {
        self.decode(buf)
      }
    */
    fn framed<T: AsyncRead + AsyncWrite + Sized>(self, io: T) -> tokio_codec::Framed<T, Self>
    where
        Self: tokio_codec::Encoder + Sized,
    {
        tokio_codec::Framed::new(io, self)
    }
}

// --

pub trait Servant {
    type Codec;

    fn addr(&self) -> std::net::SocketAddr;
    fn codec(&self) -> Self::Codec;
    fn take_over(&self, msg: Vec<u8>) -> Option<Vec<u8>>;
    fn notify(&self) -> Box<dyn Stream<Item = Vec<u8>, Error = std::io::Error> + std::marker::Send>;

}

pub fn start_client<T>(c: &T) -> bool
where
    T: Servant
{
    let addr = c.addr();
    let stream2 = c.notify();
    let client = TcpStream::connect(&addr)
        .and_then(move |stream| {
            println!("created stream");

            let (wt, rd) = RecordCodec::new().framed(stream).split();

            let interval_fut = stream2.forward(wt).then(|result| {
                println!("wrote to stream; success={:?}", result.is_ok());
                Ok(())
            });

            let socket_reader = rd.for_each(move |msg| {
                println!("recv: {:?}", msg);
                Ok(())
            });

            let connection = interval_fut.select(socket_reader);
            connection.then(move |_| {
                println!("Connection closed.");
                Ok(())
            })
        })
        .map_err(|err| {
            println!("connection error = {:?}", err);
        });

    // Following snippets come here...
    println!("About to create the stream and write to it...");
    tokio::run(client);
    println!("Stream has been created and written to.");

    true
}


pub struct Client;

impl Servant for Client {
    type Codec = RecordCodec;
    // type Item = <RecordCodec as Decoder>::Item;
    // type Error = <RecordCodec as Decoder>::Error;

    fn addr(&self) -> std::net::SocketAddr {
        let addr = "127.0.0.1:6142".to_string();
        addr.parse().unwrap()
    }

    fn codec(&self) -> Self::Codec {
        RecordCodec::new()
    }

    fn take_over(&self, msg: Vec<u8>) -> Option<Vec<u8>> {
        if msg.is_empty() {
            return None;
        }
        Some(msg)
    }

    fn notify(&self) -> Box<dyn Stream<Item = Vec<u8>, Error = std::io::Error> + std::marker::Send> {
        // Box::new(stream::iter_ok::<_, Self::Error>(vec![vec![17], vec![19]]))
        let mut n = 0_usize;
        let interval = Interval::new(Instant::now(), Duration::from_secs(1))
            .take(8)
            .map_err(bad_utf8)
            .map(move |_| {
                n %= 26;
                n += 2;
                gen_vec2(n)
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
