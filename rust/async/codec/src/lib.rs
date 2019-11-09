extern crate bytes;
extern crate futures_codec;

// --

mod utility;
mod length_codec;
mod aead_codec;

// --

pub use length_codec::LengthCodec;

