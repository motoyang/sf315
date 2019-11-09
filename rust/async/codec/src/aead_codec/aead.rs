// -- aead.rs --

use bytes::{Buf, BufMut, Bytes, BytesMut, IntoBuf};
use futures_codec::{Decoder, Encoder};
use ring::{aead::*, error::Unspecified, hkdf::*, rand::*};
use std::{cell::RefCell, sync::Arc};

// --

const SALT_LEN: usize = 12;
const TAG_LEN: usize = 16;
const PADDING_LEN: u8 = 128;
const NONCE_LEN: usize = 12;
const APP_INFO: &[u8] = b"bee lib";
const PSK: &[u8] = b"abcdefg";

// --

struct Sequence([u8; NONCE_LEN]);

impl Sequence {
    fn new() -> Self {
        assert_eq!(TAG_LEN, CHACHA20_POLY1305.tag_len());
        assert_eq!(NONCE_LEN, CHACHA20_POLY1305.nonce_len());

        Sequence([0; NONCE_LEN])
    }
}

impl NonceSequence for Sequence {
    fn advance(&mut self) -> Result<Nonce, Unspecified> {
        assert_eq!(
            std::mem::size_of::<u32>() + std::mem::size_of::<u64>(),
            std::mem::size_of_val(&self.0)
        );

        let ptr = self.0.as_mut_ptr();
        let p_u32 = ptr as *mut u32;
        let p_u64: *mut u64;
        unsafe {
            p_u64 = ptr.offset(std::mem::size_of::<u32>() as isize) as *mut u64;
            *p_u64 += 1;
            if *p_u64 == u64::max_value() {
                *p_u64 = 0;

                *p_u32 += 1;
                if *p_u32 == u32::max_value() {
                    *p_u32 = 0;
                }
            }
        }
        Nonce::try_assume_unique_for_key(&self.0)
    }
}

// --

fn get_padding() -> Vec<u8> {
    let rng = SystemRandom::new();
    let mut padding_len = crate::utility::no_zero_rand_gen::<u8>(&rng);
    padding_len %= PADDING_LEN;
    padding_len += 1;

    let mut v = Vec::<u8>::new();
    v.push(padding_len);
    v.resize(padding_len as usize, 0);
    v
}

// --

#[derive(Clone)]
pub struct AeadCodec {
    sealing_key: Option<Arc<RefCell<SealingKey<Sequence>>>>,
    opening_key: Option<Arc<RefCell<OpeningKey<Sequence>>>>,
    body_len: Option<usize>,
}

impl AeadCodec {
    fn derive_encode_key(&mut self, buf: &mut BytesMut) {
        assert!(self.sealing_key.is_none());

        let mut salt = [0_u8; SALT_LEN];
        let rng = SystemRandom::new();
        rng.fill(&mut salt).unwrap();
        buf.put_slice(&salt);

        let salt = Salt::new(HKDF_SHA256, &salt);
        let prk = salt.extract(PSK);
        let okm = prk.expand(&[APP_INFO], &CHACHA20_POLY1305).unwrap();
        let ubk = UnboundKey::from(okm);
        self.sealing_key = Some(Arc::new(RefCell::new(SealingKey::new(
            ubk,
            Sequence::new(),
        ))));
    }

    fn derive_decode_key(&mut self, buf: &mut BytesMut) {
        assert!(self.opening_key.is_none());

        let salt = buf.split_to(SALT_LEN);
        let salt = Salt::new(HKDF_SHA256, &salt);
        let prk = salt.extract(PSK);
        let okm = prk.expand(&[APP_INFO], &CHACHA20_POLY1305).unwrap();
        let ubk = UnboundKey::from(okm);
        self.opening_key = Some(Arc::new(RefCell::new(OpeningKey::new(
            ubk,
            Sequence::new(),
        ))));
    }

    fn sealing_encode(&self, buf: &mut BytesMut) {
        self.sealing_key
            .as_ref()
            .unwrap()
            .borrow_mut()
            .seal_in_place_append_tag(Aad::empty(), buf)
            .unwrap();
    }

    fn opening_decode<'a>(&self, buf: &'a mut BytesMut) -> &'a mut [u8] {
        self.opening_key
            .as_ref()
            .unwrap()
            .borrow_mut()
            .open_in_place(Aad::empty(), buf)
            .unwrap()
    }
}

impl Encoder for AeadCodec {
    type Item = Vec<u8>;
    type Error = std::io::Error;

    fn encode(&mut self, line: Self::Item, buf: &mut BytesMut) -> Result<(), Self::Error> {
        let mut body_len = line.len();
        if body_len == 0 {
            return Ok(());
        }
        if body_len > u16::max_value() as usize {
            return Err(std::io::Error::new(
                std::io::ErrorKind::InvalidData,
                "Input line is too long.",
            ));
        }

        let padding = get_padding();
        body_len += padding.len();

        let mut head_len = std::mem::size_of::<u16>();
        head_len += TAG_LEN;
        body_len += TAG_LEN;
        if self.sealing_key.is_none() {
            buf.reserve(SALT_LEN + head_len + body_len);
            self.derive_encode_key(buf);
        } else {
            buf.reserve(head_len + body_len);
        }

        let mut buf2 = buf.split_off(buf.len());
        buf2.put_u16_be(body_len as u16);
        self.sealing_encode(&mut buf2);
        buf.extend_from_slice(&buf2);

        buf2 = buf.split_off(buf.len());
        buf2.put_slice(&line);
        buf2.put_slice(&padding);
        self.sealing_encode(&mut buf2);
        buf.extend_from_slice(&buf2);

        Ok(())
    }
}

impl Decoder for AeadCodec {
    type Item = Bytes;
    type Error = std::io::Error;

    fn decode(&mut self, buf: &mut BytesMut) -> Result<Option<Self::Item>, Self::Error> {
        if self.opening_key.is_none() {
            self.derive_decode_key(buf);
        }

        let head_len = std::mem::size_of::<u16>() + TAG_LEN;
        if self.body_len == None && buf.len() >= head_len {
            let mut head = buf.split_to(head_len);
            let head = self.opening_decode(&mut head);
            let mut head = Bytes::from(head as &[u8]).into_buf();
            self.body_len = Some(head.get_u16_be() as usize);
        }

        if let Some(body_len) = self.body_len {
            if buf.len() >= body_len {
                self.body_len = None;
                let mut body = buf.split_to(body_len);
                let body = self.opening_decode(&mut body);
                let padding_len = *body.iter().rev().find(|&&v| v > 0).unwrap();
                let (body, _) = body.split_at(body.len() - (padding_len as usize));
                Ok(Some(Bytes::from(body as &[u8])))
            } else {
                Ok(None)
            }
        } else {
            Ok(None)
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn it_works() {
        assert_eq!(2 + 2, 4);

        let h = b"hello";
        let mut bytes = BytesMut::new();
        println!("cap = {}", bytes.capacity());

        bytes.put_slice(h);
        println!("bytes = {:?}", bytes);

        let e = bytes.iter().rev().find(|&s| *s == 101);
        assert_eq!(*e.unwrap(), 101);
    }

    #[test]
    fn t_get_padding() {
        for _ in 0..8 {
            let padding = get_padding();
            assert_eq!(padding[0] as usize, padding.len());
            println!("padding = {:?}", padding);
        }
    }
    #[test]
    fn t_sequence() {
        let mut seq = Sequence::new();
        for i in 1..11 {
            seq.advance().unwrap();
            println!("{:?}", seq.0);
            let ptr = seq.0.as_ptr() as *const u8;
            unsafe {
                let ptr = ptr.offset(std::mem::size_of::<u32>() as isize) as *const usize;
                println!("*ptr = {}", *ptr);
                assert_eq!(i, *ptr);
            }
        }
    }
}
