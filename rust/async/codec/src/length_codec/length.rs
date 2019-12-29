// -- length.rs --

use {
    // byteorder::{BigEndian, ByteOrder},
    bytes::{Buf, Bytes, BytesMut},
    futures_codec::{Decoder, Encoder},
    std::io::Error,
    crate::utility::Length,
};

// --

#[derive(Default, Clone, Copy)]
pub struct LengthCodec<T: Length>(std::marker::PhantomData<T>);
/*
unsafe impl<T: Length> Send for LengthCodec<T> {}
unsafe impl<T: Length> Sync for LengthCodec<T> {}
impl<T: Length> Unpin for LengthCodec<T> {}
impl<T: Length> std::panic::UnwindSafe for LengthCodec<T> {}
impl<T: Length> std::panic::RefUnwindSafe for LengthCodec<T> {}
*/
impl<T: Length> Encoder for LengthCodec<T> {
    type Item = Bytes;
    type Error = Error;

    fn encode(&mut self, src: Self::Item, dst: &mut BytesMut) -> Result<(), Self::Error> {
        let head_len = std::mem::size_of_val(&T::from_usize(Default::default()));
        dst.reserve(head_len + src.len());
        T::from_usize(src.len()).put(dst);
        dst.extend_from_slice(&src);
        Ok(())
    }
}

impl<T: Length> Decoder for LengthCodec<T> {
    type Item = Bytes;
    type Error = Error;

    fn decode(&mut self, src: &mut BytesMut) -> Result<Option<Self::Item>, Self::Error> {
        let head_len = std::mem::size_of_val(&T::from_usize(Default::default()));
        if src.len() < head_len {
            return Ok(None);
        }

        let len: usize = T::get(&src[..head_len]).to_usize();
        if src.len() - head_len >= len {
            src.advance(head_len);
            Ok(Some(src.split_to(len).freeze()))
        } else {
            Ok(None)
        }
    }
}

// --

#[cfg(test)]
mod tests {
    extern crate test_case;

    use crate::utility::type_of;
    use super::*;
    use futures::{executor, io::Cursor, sink::SinkExt, TryStreamExt};
    use futures_codec::{BytesCodec, Framed, FramedRead, FramedWrite};
    use test_case::test_case;

    // --

    #[test_case(Bytes::from("Hello World!") => vec![72, 101, 108, 108, 111, 32, 87, 111, 114, 108, 100, 33]; "hello")]
    #[test_case(Bytes::from("this is a new world!") => vec![116, 104, 105, 115, 32, 105, 115, 32, 97, 32, 110, 101, 119, 32, 119, 111, 114, 108, 100, 33]; "new_world")]
    fn bytes_codec_encode(msg: Bytes) -> Vec<u8> {
        executor::block_on(async move {
            let mut buf = vec![];
            let cur = Cursor::new(&mut buf);
            let mut framed = Framed::new(cur, BytesCodec {});

            framed.send(msg.clone()).await.unwrap();
            println!("\nbuf: {} = {:?}", type_of(&&buf), buf);
            buf
        })
    }

    #[test_case(vec![72, 101, 108, 108, 111, 32, 87, 111, 114, 108, 100, 33] => Bytes::from("Hello World!"); "hello")]
    #[test_case(vec![116, 104, 105, 115, 32, 105, 115, 32, 97, 32, 110, 101, 119, 32, 119, 111, 114, 108, 100, 33] => Bytes::from("this is a new world!"); "new_world")]
    fn bytes_codec_decode(mut buf: Vec<u8>) -> Bytes {
        executor::block_on(async move {
            let cur = Cursor::new(&mut buf);
            let mut framed = Framed::new(cur, BytesCodec {});
            if let Some(msg) = framed.try_next().await.unwrap() {
                println!("\nmsg: {} = {:?}", type_of(&msg), msg);
                msg
            } else {
                Bytes::new()
            }
        })
    }

    #[test]
    fn t_bytes_codec() {
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
    }

    #[test]
    fn t_length_u8_codec() {
        executor::block_on(async move {
            let mut buf = vec![];
            let cur = Cursor::new(&mut buf);
            let mut framed = FramedWrite::new(cur, LengthCodec::<u8>::default());

            let msg = Bytes::from("Hello World!");
            framed.send(msg.clone()).await.unwrap();
            println!("buf: {:?}", buf);

            let mut framed2 = FramedRead::new(&buf[..], LengthCodec::<u8>::default());
            let msg2 = framed2.try_next().await.unwrap().unwrap();
            println!("msg: {:?}", msg2);

            assert_eq!(msg, msg2);
        });
    }
    #[test]
    fn t_length_u16_codec() {
        executor::block_on(async move {
            let mut buf = vec![];
            let cur = Cursor::new(&mut buf);
            let mut framed = FramedWrite::new(cur, LengthCodec::<u16>::default());

            let msg = Bytes::from("Hello World!");
            framed.send(msg.clone()).await.unwrap();
            println!("buf: {:?}", buf);

            let mut framed2 = FramedRead::new(&buf[..], LengthCodec::<u16>::default());
            let msg2 = framed2.try_next().await.unwrap().unwrap();
            println!("msg: {:?}", msg2);

            assert_eq!(msg, msg2);
        });
    }
    #[test]
    fn t_length_u32_codec() {
        executor::block_on(async move {
            let mut buf = vec![];
            let cur = Cursor::new(&mut buf);
            let mut framed = FramedWrite::new(cur, LengthCodec::<u32>::default());

            let msg = Bytes::from("Hello World!");
            framed.send(msg.clone()).await.unwrap();
            println!("buf: {:?}", buf);

            let mut framed2 = FramedRead::new(&buf[..], LengthCodec::<u32>::default());
            let msg2 = framed2.try_next().await.unwrap().unwrap();
            println!("msg: {:?}", msg2);

            assert_eq!(msg, msg2);
        });
    }
    #[test]
    fn t_length_u64_codec() {
        executor::block_on(async move {
            let mut buf = vec![];
            let cur = Cursor::new(&mut buf);
            let mut framed = FramedWrite::new(cur, LengthCodec::<u64>::default());

            let msg = Bytes::from("Hello World!");
            framed.send(msg.clone()).await.unwrap();
            println!("buf: {:?}", buf);

            let mut framed2 = FramedRead::new(&buf[..], LengthCodec::<u64>::default());
            let msg2 = framed2.try_next().await.unwrap().unwrap();
            println!("msg: {:?}", msg2);

            assert_eq!(msg, msg2);
        });
    }
}
