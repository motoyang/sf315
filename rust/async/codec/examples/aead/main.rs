extern crate bytes;
extern crate futures;
extern crate futures_codec;

// --

use bytes::Bytes;
use futures::{executor, io::Cursor, sink::SinkExt, StreamExt, TryStreamExt};
use futures_codec::{BytesCodec, Decoder, Encoder, Framed, FramedRead, FramedWrite};
use std::io::Error;

use codec::{AeadCodecBuilder as Builder, AES_256_GCM, HKDF_SHA512};

fn main() {
    executor::block_on(async move {
        let buf = b"Hello World!";
        let mut framed = FramedRead::new(&buf[..], BytesCodec {});

        let msg = framed.try_next().await.unwrap().unwrap();
        println!("msg: {:?}", msg);
        assert_eq!(msg, Bytes::from(&buf[..]));
    });

    let psk = "thisispsk";

    let mut builder = Builder::default();
    executor::block_on(codec(
        builder.clone().create(psk),
        builder.clone().create(psk),
    ));

    builder
        .set_salt_len(24)
        .set_padding_len(64)
        .set_aead_algorithm(&AES_256_GCM)
        .set_hkdf_algorithm(&HKDF_SHA512);
    executor::block_on(codec2(
        builder.clone().create(psk),
        builder.clone().create(psk),
    ));
}

async fn codec<T>(c: T, d: T)
where
    T: Encoder<Item = Bytes, Error = Error> + Decoder<Item = Bytes, Error = Error> + Clone,
{
    let mut buf = vec![];
    let cur = Cursor::new(&mut buf);
    let mut framed = FramedWrite::new(cur, c);

    let mut i = 0_usize;
    while {
        i += 1;
        let msg = Bytes::from(format!("Hello World! #{}", i));
        framed.send(msg.clone()).await.unwrap();

        i < 88
    } {}
    println!("buf: {:?}", buf);

    i = 0;
    let mut framed2 = FramedRead::new(&buf[..], d);
    while let Some(msg2) = framed2.next().await {
        let msg2 = msg2.unwrap();
        println!("msg: {:?}", msg2);

        i += 1;
        assert_eq!(msg2, Bytes::from(format!("Hello World! #{}", i)));
    }
}

async fn codec2<T>(c: T, d: T)
where
    T: Encoder<Item = Bytes, Error = Error> + Decoder<Item = Bytes, Error = Error> + Clone,
{
    let mut buf = vec![];
    let cur = Cursor::new(&mut buf);
    let mut framed = Framed::new(cur, c);

    let mut i = 0_usize;
    while {
        i += 1;
        let msg = Bytes::from(format!("Hello Customer! #{}", i));
        framed.send(msg.clone()).await.unwrap();

        i < 68
    } {}
    println!("buf: {:?}", buf);

    i = 0;
    let cur = Cursor::new(&mut buf);
    let mut framed2 = Framed::new(cur, d);
    while let Some(msg2) = framed2.try_next().await.unwrap() {
        println!("msg: {:?}", msg2);

        i += 1;
        assert_eq!(msg2, Bytes::from(format!("Hello Customer! #{}", i)));
    }
}
