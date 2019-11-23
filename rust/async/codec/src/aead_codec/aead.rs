// -- aead.rs --

use {
    bytes::{Buf, BufMut, Bytes, BytesMut, IntoBuf},
    futures_codec::{Decoder, Encoder},
    ring::{
        aead::{
            Aad, Algorithm as AeadAlgorithm, BoundKey, Nonce, NonceSequence, OpeningKey,
            SealingKey, UnboundKey, CHACHA20_POLY1305,
        },
        error::Unspecified,
        hkdf::{Algorithm as HkdfAlgorithm, Salt, HKDF_SHA256},
        rand::*,
    },
    std::{cell::RefCell, rc::Rc},
};

// --

const APP_INFO: &[u8] = b"bee lib";

// --

struct Sequence(Vec<u8>);

impl Sequence {
    fn new(nonce_len: usize) -> Self {
        Sequence(vec![0; nonce_len])
    }
}

impl NonceSequence for Sequence {
    fn advance(&mut self) -> Result<Nonce, Unspecified> {
        assert_eq!(
            std::mem::size_of::<u32>() + std::mem::size_of::<u64>(),
            self.0.len()
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

#[derive(Clone)]
pub struct Builder {
    hkdf_algorithm: &'static HkdfAlgorithm,
    aead_algorithm: &'static AeadAlgorithm,
    salt_len: usize,
    padding_len: u8,
}

impl Default for Builder {
    fn default() -> Self {
        Self {
            hkdf_algorithm: &HKDF_SHA256,
            aead_algorithm: &CHACHA20_POLY1305,
            salt_len: 12,
            padding_len: 128,
        }
    }
}

impl Builder {
    pub fn set_hkdf_algorithm(&mut self, a: &'static HkdfAlgorithm) -> &mut Self {
        self.hkdf_algorithm = a;
        self
    }
    pub fn set_aead_algorithm(&mut self, a: &'static AeadAlgorithm) -> &mut Self {
        self.aead_algorithm = a;
        self
    }
    pub fn set_salt_len(&mut self, len: usize) -> &mut Self {
        self.salt_len = len;
        self
    }
    pub fn set_padding_len(&mut self, len: u8) -> &mut Self {
        self.padding_len = len;
        self
    }
    pub fn create(self, psk: &str) -> AeadCodec {
        AeadCodec::new(self, psk)
    }
}

// --

#[derive(Clone)]
pub struct AeadCodec {
    builder: Rc<Builder>,
    psk: Rc<String>,
    sealing_key: Option<Rc<RefCell<SealingKey<Sequence>>>>,
    opening_key: Option<Rc<RefCell<OpeningKey<Sequence>>>>,
    body_len: Option<usize>,
}

impl AeadCodec {
    fn new(builder: Builder, psk: &str) -> Self {
        Self {
            builder: Rc::new(builder),
            psk: Rc::new(String::from(psk)),
            sealing_key: None,
            opening_key: None,
            body_len: None,
        }
    }

    fn get_padding(&self) -> Vec<u8> {
        let rng = SystemRandom::new();
        let mut padding_len = crate::utility::no_zero_rand_gen::<u8>(&rng);
        padding_len %= self.builder.padding_len;
        padding_len += 1;

        let mut v = Vec::<u8>::new();
        v.push(padding_len);
        v.resize(padding_len as usize, 0);
        v
    }

    fn derive_encode_key(&mut self, buf: &mut BytesMut) {
        assert!(self.sealing_key.is_none());

        let mut salt = vec![0_u8; self.builder.salt_len];
        let rng = SystemRandom::new();
        rng.fill(&mut salt).unwrap();
        buf.put_slice(&salt);

        let salt = Salt::new(*self.builder.hkdf_algorithm, &salt);
        let prk = salt.extract(self.psk.as_bytes());
        let okm = prk
            .expand(&[APP_INFO], self.builder.aead_algorithm)
            .unwrap();
        let ubk = UnboundKey::from(okm);
        let nonce_len = self.builder.aead_algorithm.nonce_len();
        self.sealing_key = Some(Rc::new(RefCell::new(SealingKey::new(
            ubk,
            Sequence::new(nonce_len),
        ))));
    }

    fn derive_decode_key(&mut self, buf: &mut BytesMut) {
        assert!(self.opening_key.is_none());

        let salt = buf.split_to(self.builder.salt_len);
        let salt = Salt::new(*self.builder.hkdf_algorithm, &salt);
        let prk = salt.extract(self.psk.as_bytes());
        let okm = prk
            .expand(&[APP_INFO], self.builder.aead_algorithm)
            .unwrap();
        let ubk = UnboundKey::from(okm);
        let nonce_len = self.builder.aead_algorithm.nonce_len();
        self.opening_key = Some(Rc::new(RefCell::new(OpeningKey::new(
            ubk,
            Sequence::new(nonce_len),
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
    type Item = Bytes;
    type Error = std::io::Error;

    fn encode(&mut self, line: Self::Item, buf: &mut BytesMut) -> Result<(), Self::Error> {
        let mut body_len = line.len();
        if body_len == 0 {
            return Ok(());
        }
        if body_len > u32::max_value() as usize {
            return Err(std::io::Error::new(
                std::io::ErrorKind::InvalidData,
                "Input line is too long.",
            ));
        }

        let padding = self.get_padding();
        body_len += padding.len();

        let mut head_len = std::mem::size_of::<u32>();
        let tag_len = self.builder.aead_algorithm.tag_len();
        head_len += tag_len;
        body_len += tag_len;
        if self.sealing_key.is_none() {
            let salt_len = self.builder.salt_len;
            buf.reserve(salt_len + head_len + body_len);
            self.derive_encode_key(buf);
        } else {
            buf.reserve(head_len + body_len);
        }

        let mut buf2 = buf.split_off(buf.len());
        buf2.put_u32_be(body_len as u32);
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
            let salt_len = self.builder.salt_len;
            if buf.len() >= salt_len {
                self.derive_decode_key(buf);
            } else {
                return Ok(None);
            }
        }

        let tag_len = self.builder.aead_algorithm.tag_len();
        let head_len = std::mem::size_of::<u32>() + tag_len;
        if self.body_len == None && buf.len() >= head_len {
            let mut head = buf.split_to(head_len);
            let head = self.opening_decode(&mut head);
            let mut head = Bytes::from(head as &[u8]).into_buf();
            self.body_len = Some(head.get_u32_be() as usize);
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

// --

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
        let builder = Builder::default();
        let codec = builder.create("abcd");
        println!("");
        for _ in 0..8 {
            let padding = codec.get_padding();
            assert_eq!(padding[0] as usize, padding.len());
            padding[1..].iter().for_each(|x| assert_eq!(*x, 0));
            println!("padding = {:?}", padding);
        }
    }
    #[test]
    fn t_sequence() {
        let nonce_len = 12_usize;
        let mut seq = Sequence::new(nonce_len);
        println!("");
        for i in 1..11 {
            seq.advance().unwrap();
            println!("{:?}", seq.0);
            let ptr = seq.0.as_ptr() as *const u8;
            unsafe {
                let ptr = ptr.offset(std::mem::size_of::<u32>() as isize) as *const usize;
                println!("*ptr = {}", *ptr);
                assert_eq!(i, *ptr);
            };
        }
    }
}
