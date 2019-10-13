// -- record.rs --

use bytes::{Buf, BufMut, Bytes, BytesMut, IntoBuf};
use tokio_codec::{Decoder, Encoder};

// --

#[derive(Copy, Clone)]
pub struct RecordCodec;

impl RecordCodec {
  pub fn new() -> RecordCodec {
    RecordCodec
  }
}

impl Encoder for RecordCodec {
  type Item = Vec<u8>;
  type Error = std::io::Error;

  fn encode(&mut self, line: Self::Item, buf: &mut BytesMut) -> Result<(), Self::Error> {
    let body_len = line.len();
    if body_len == 0 {
      return Ok(());
    }

    let head_len = std::mem::size_of::<u16>();
    buf.reserve(body_len + head_len);
    buf.put_u16_le(body_len as u16);
    buf.put(line);
    Ok(())
  }
}

impl Decoder for RecordCodec {
  type Item = Bytes;
  type Error = std::io::Error;

  // Find the next line in buf!
  fn decode(&mut self, buf: &mut BytesMut) -> Result<Option<Self::Item>, Self::Error> {
    let head_len = std::mem::size_of::<u16>();
    let buf_len = buf.len();
    let mut b = buf.clone().freeze().into_buf();
    let body_len = if buf_len > head_len {
      b.get_u16_le() as usize
    } else {
      return Ok(None);
    };

    Ok(if body_len + head_len <= buf_len {
      let b = buf.split_to(head_len + body_len).split_off(head_len);
      Some(b.freeze())
    } else {
      None
    })
  }
}
