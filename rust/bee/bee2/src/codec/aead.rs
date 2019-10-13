// -- aead.rs --

use bytes::{Buf, BufMut, Bytes, BytesMut, IntoBuf};
use log::trace;
use ring::{aead::*, error::Unspecified, hkdf::*, rand::*};
use std::{
    collections::HashMap,
    sync::Mutex,
};
use tokio_codec::{Decoder, Encoder};

// --

lazy_static! {
    static ref AEAD_IN_MUTEX: Mutex<AeadInMap> = { Mutex::new(AeadInMap::new()) };
}

// --

struct AeadInMap {
    current: usize,
    sealing_map: HashMap<usize, SealingKey<Sequence>>,
    opening_map: HashMap<usize, OpeningKey<Sequence>>,
}

impl AeadInMap {
    fn new() -> Self {
        Self {
            current: 0,
            sealing_map: HashMap::new(),
            opening_map: HashMap::new(),
        }
    }

    fn insert_sealing(&mut self, sealing: SealingKey<Sequence>) -> usize {
        self.current += 1;
        self.sealing_map.insert(self.current, sealing);
        trace!("insert sealing key at index: {}.", self.current);
        self.current
    }

    fn remove_sealing(&mut self, index: usize) {
        self.sealing_map.remove(&index);
        trace!("remove sealing key from AEAD_IN_MUTEX.")
    }

    fn insert_opening(&mut self, opening: OpeningKey<Sequence>) -> usize {
        self.current += 1;
        self.opening_map.insert(self.current, opening);
        trace!("insert opening key at index: {}.", self.current);
        self.current
    }

    fn remove_opening(&mut self, index: usize) {
        self.opening_map.remove(&index);
        trace!("remove opening key from AEAD_IN_MUTEX.")
    }
}

// --

const SALT_LEN: usize = 12;
const TAG_LEN: usize = 16;
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

#[derive(Copy, Clone)]
pub struct AeadCodec {
    sealing_index: usize,
    opening_index: usize,
    body_len: Option<usize>,
}

impl AeadCodec {
    fn derive_encode_key(&mut self, buf: &mut BytesMut) {
        let mut salt = [0_u8; SALT_LEN];
        let rng = SystemRandom::new();
        rng.fill(&mut salt).unwrap();
        buf.put_slice(&salt);

        let salt = Salt::new(HKDF_SHA256, &salt);
        let prk = salt.extract(PSK);
        let okm = prk.expand(&[APP_INFO], &CHACHA20_POLY1305).unwrap();
        let ubk = UnboundKey::from(okm);
        let sealing = SealingKey::new(ubk, Sequence::new());

        let mut aead_in_map = AEAD_IN_MUTEX.lock().unwrap();
        self.sealing_index = aead_in_map.insert_sealing(sealing);
    }

    fn derive_decode_key(&mut self, buf: &mut BytesMut) {
        let salt = buf.split_to(SALT_LEN);
        let salt = Salt::new(HKDF_SHA256, &salt);
        let prk = salt.extract(PSK);
        let okm = prk.expand(&[APP_INFO], &CHACHA20_POLY1305).unwrap();
        let ubk = UnboundKey::from(okm);
        let opening = OpeningKey::new(ubk, Sequence::new());

        let mut aead_in_map = AEAD_IN_MUTEX.lock().unwrap();
        self.opening_index =  aead_in_map.insert_opening(opening);
    }

    fn sealing_encode(&self, buf: &mut BytesMut) {
        let mut aead_in_map = AEAD_IN_MUTEX.lock().unwrap();
        let sealing_key = aead_in_map
            .sealing_map
            .get_mut(&self.sealing_index)
            .unwrap();
        sealing_key
            .seal_in_place_append_tag(Aad::empty(), buf)
            .unwrap();
    }

    fn opening_decode<'a>(&self, buf: &'a mut BytesMut) -> &'a mut [u8] {
        let aead_in_map = &mut *AEAD_IN_MUTEX.lock().unwrap();
        let opening_key = aead_in_map
            .opening_map
            .get_mut(&self.opening_index)
            .unwrap();
        opening_key.open_in_place(Aad::empty(), buf).unwrap()
    }
}

impl super::super::app::NewDrop for AeadCodec {
    fn new() -> Self {
        Self {
            sealing_index: 0,
            opening_index: 0,
            body_len: None,
        }
    }

    fn drop(&mut self) {
        let mut aead_in_map = AEAD_IN_MUTEX.lock().unwrap();
        aead_in_map.remove_sealing(self.sealing_index);
        aead_in_map.remove_opening(self.opening_index);
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

        let mut head_len = std::mem::size_of::<u16>();
        head_len += TAG_LEN;
        body_len += TAG_LEN;
        if self.sealing_index == 0 {
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
        self.sealing_encode(&mut buf2);
        buf.extend_from_slice(&buf2);

        Ok(())
    }
}

impl Decoder for AeadCodec {
    type Item = Bytes;
    type Error = std::io::Error;

    fn decode(&mut self, buf: &mut BytesMut) -> Result<Option<Self::Item>, Self::Error> {
        if self.opening_index == 0 {
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
                Ok(Some(Bytes::from(body as &[u8])))
            } else {
                Ok(None)
            }
        } else {
            Ok(None)
        }
    }
}
