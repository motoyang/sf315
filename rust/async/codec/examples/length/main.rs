extern crate bytes;
extern crate futures;
extern crate futures_codec;

// --

use bytes::Bytes;
use futures::{executor, io::Cursor, sink::SinkExt, TryStreamExt};
use futures_codec::{BytesCodec, Decoder, Encoder, Framed, FramedRead, FramedWrite};
use std::io::Error;

use codec::LengthCodec;

fn main() {
    executor::block_on(async move {
        let mut buf = vec![];
        let cur = Cursor::new(&mut buf);
        let mut framed = Framed::new(cur, BytesCodec {});

        let msg = Bytes::from("Hello World!");
        framed.send(msg.clone()).await.unwrap();
        println!("buf: {:?}", buf);

        let cur = Cursor::new(&mut buf);
        let mut framed2 = Framed::new(cur, BytesCodec {});
        while let Some(msg2) = framed2.try_next().await.unwrap() {
            println!("msg: {:?}", msg2);
            assert_eq!(msg, msg2);
        }
    });

    executor::block_on(codec(LengthCodec::<u8>::new()));
    executor::block_on(codec(LengthCodec::<u16>::new()));
    executor::block_on(codec(LengthCodec::<u32>::new()));
    executor::block_on(codec(LengthCodec::<u64>::new()));

    executor::block_on(codec2(LengthCodec::<u8>::new()));
}

async fn codec<T>(c: T)
where
    T: Encoder<Item = Bytes, Error = Error> + Decoder<Item = Bytes, Error = Error> + Copy,
{
    let mut buf = vec![];
    // Cursor implements AsyncRead and AsyncWrite
    let cur = Cursor::new(&mut buf);
    let mut framed = FramedWrite::new(cur, c);

    let msg = Bytes::from("Hello World!");
    framed.send(msg.clone()).await.unwrap();
    println!("buf: {:?}", buf);

    let mut framed2 = FramedRead::new(&buf[..], c);
    let msg2 = framed2.try_next().await.unwrap().unwrap();
    println!("msg: {:?}", msg2);

    assert_eq!(msg, msg2);
}

async fn codec2<T>(c: T)
where
    T: Encoder<Item = Bytes, Error = Error> + Decoder<Item = Bytes, Error = Error> + Copy,
{
    let mut buf = vec![];
    // Cursor implements AsyncRead and AsyncWrite
    let cur = Cursor::new(&mut buf);
    let mut framed = Framed::new(cur, c);

    let msg = Bytes::from("Hello World!");
    framed.send(msg.clone()).await.unwrap();
    println!("buf: {:?}", buf);

    let cur = Cursor::new(&mut buf);
    let mut framed2 = Framed::new(cur, c);
    let msg2 = framed2.try_next().await.unwrap().unwrap();
    println!("msg: {:?}", msg2);

    assert_eq!(msg, msg2);
}
