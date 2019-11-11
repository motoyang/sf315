// -- length.rs --

use {
    bytes::{BigEndian, BufMut, ByteOrder, Bytes, BytesMut},
    futures_codec::{Decoder, Encoder},
    std::io::Error,
};

// --

pub trait Length {
    fn put(&self, dst: &mut BytesMut);
    fn get(src: &[u8]) -> Self;
    fn from_usize(len: usize) -> Self;
    fn to_usize(&self) -> usize;
}

impl Length for u8 {
    #[inline]
    fn put(&self, dst: &mut BytesMut) {
        dst.put_u8(*self);
    }
    #[inline]
    fn get(src: &[u8]) -> Self {
        src[0]
    }
    #[inline]
    fn from_usize(len: usize) -> Self {
        assert!(len <= Self::max_value() as usize);
        len as Self
    }
    #[inline]
    fn to_usize(&self) -> usize {
        *self as usize
    }
}

impl Length for u16 {
    #[inline]
    fn put(&self, dst: &mut BytesMut) {
        dst.put_u16_be(*self);
    }
    #[inline]
    fn get(src: &[u8]) -> Self {
        BigEndian::read_u16(src)
    }
    #[inline]
    fn from_usize(len: usize) -> Self {
        assert!(len <= Self::max_value() as usize);
        len as Self
    }
    #[inline]
    fn to_usize(&self) -> usize {
        *self as usize
    }
}

impl Length for u32 {
    #[inline]
    fn put(&self, dst: &mut BytesMut) {
        dst.put_u32_be(*self);
    }
    #[inline]
    fn get(src: &[u8]) -> Self {
        BigEndian::read_u32(src)
    }
    #[inline]
    fn from_usize(len: usize) -> Self {
        assert!(len <= Self::max_value() as usize);
        len as Self
    }
    #[inline]
    fn to_usize(&self) -> usize {
        *self as usize
    }
}

impl Length for u64 {
    #[inline]
    fn put(&self, dst: &mut BytesMut) {
        dst.put_u64_be(*self);
    }
    #[inline]
    fn get(src: &[u8]) -> Self {
        BigEndian::read_u64(src)
    }
    #[inline]
    fn from_usize(len: usize) -> Self {
        assert!(len <= Self::max_value() as usize);
        len as Self
    }
    #[inline]
    fn to_usize(&self) -> usize {
        *self as usize
    }
}

// --

#[derive(Clone, Copy)]
pub struct LengthCodec<T: Length + Default + Copy>([T; 0]);

impl<T: Length + Default + Copy> LengthCodec<T> {
    pub fn new() -> Self {
        Self([Default::default(); 0])
    }
}

impl<T: Length + Default + Copy> Encoder for LengthCodec<T> {
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

impl<T: Length + Default + Copy> Decoder for LengthCodec<T> {
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
    use super::*;
    use futures::{executor, io::Cursor, sink::SinkExt, TryStreamExt};
    use futures_codec::{BytesCodec, Framed, FramedRead, FramedWrite};

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
            let mut framed = FramedWrite::new(cur, LengthCodec::<u8>::new());

            let msg = Bytes::from("Hello World!");
            framed.send(msg.clone()).await.unwrap();
            println!("buf: {:?}", buf);

            let mut framed2 = FramedRead::new(&buf[..], LengthCodec::<u8>::new());
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
            let mut framed = FramedWrite::new(cur, LengthCodec::<u16>::new());

            let msg = Bytes::from("Hello World!");
            framed.send(msg.clone()).await.unwrap();
            println!("buf: {:?}", buf);

            let mut framed2 = FramedRead::new(&buf[..], LengthCodec::<u16>::new());
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
            let mut framed = FramedWrite::new(cur, LengthCodec::<u32>::new());

            let msg = Bytes::from("Hello World!");
            framed.send(msg.clone()).await.unwrap();
            println!("buf: {:?}", buf);

            let mut framed2 = FramedRead::new(&buf[..], LengthCodec::<u32>::new());
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
            let mut framed = FramedWrite::new(cur, LengthCodec::<u64>::new());

            let msg = Bytes::from("Hello World!");
            framed.send(msg.clone()).await.unwrap();
            println!("buf: {:?}", buf);

            let mut framed2 = FramedRead::new(&buf[..], LengthCodec::<u64>::new());
            let msg2 = framed2.try_next().await.unwrap().unwrap();
            println!("msg: {:?}", msg2);

            assert_eq!(msg, msg2);
        });
    }
}
