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
