// -- length.rs --

use {
    byteorder::{BigEndian, ByteOrder},
    bytes::{BufMut, BytesMut},
};

// --

pub trait Length: Default + Copy {
    fn put(&self, dst: &mut BytesMut);
    fn get(src: &[u8]) -> Self;
    fn from_usize(len: usize) -> Self;
    fn to_usize(&self) -> usize;
    fn max() -> usize;
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
    #[inline]
    fn max() -> usize {
        Self::max_value() as usize
    }
}

impl Length for u16 {
    #[inline]
    fn put(&self, dst: &mut BytesMut) {
        dst.put_u16(*self);
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
    #[inline]
    fn max() -> usize {
        Self::max_value() as usize
    }
}

impl Length for u32 {
    #[inline]
    fn put(&self, dst: &mut BytesMut) {
        dst.put_u32(*self);
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
    #[inline]
    fn max() -> usize {
        Self::max_value() as usize
    }
}

impl Length for u64 {
    #[inline]
    fn put(&self, dst: &mut BytesMut) {
        dst.put_u64(*self);
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
    #[inline]
    fn max() -> usize {
        Self::max_value() as usize
    }
}
