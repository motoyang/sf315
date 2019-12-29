// -- lib.rs --

extern crate byteorder;
extern crate bytes;
extern crate futures_codec;
extern crate ring;
extern crate serde;
extern crate bincode;

// --

mod aead_codec;
mod length_codec;
mod record_codec;
mod utility;

// --

pub use aead_codec::{
    Builder as AeadCodecBuilder, AES_128_GCM, AES_256_GCM, CHACHA20_POLY1305, HKDF_SHA256,
    HKDF_SHA384, HKDF_SHA512,
};
pub use length_codec::LengthCodec;
pub use record_codec::RecordCodec;