// -- record.rs --

use bytes::{Buf, BufMut, Bytes, BytesMut, IntoBuf};
use tokio::sync::mpsc::error::{SendError};
use tokio_codec::{Decoder, Encoder};

// --

pub fn extract(mut b: Bytes) -> (u64, Bytes) {
    let rb = b.split_to(std::mem::size_of::<u64>());
    (rb.into_buf().get_u64_be(), b)
}

pub fn pack(id: u64, msg: Bytes) -> Bytes {
    let len = std::mem::size_of_val(&id) + msg.len();
    let mut b = BytesMut::with_capacity(len);
    b.put_u64_be(id);
    b.put_slice(&msg);
    b.freeze()
}

// --

#[derive(Debug)]
pub struct CodecError;

impl std::convert::From<std::io::Error> for CodecError {
    fn from(_: std::io::Error) -> Self {
        Self
    }
}

impl std::convert::From<SendError> for CodecError {
    fn from(_: SendError) -> Self {
        Self
    }
}

// --

pub trait NewDrop {
    fn new() -> Self;
    fn drop(&mut self);
}

#[derive(Copy, Clone, Debug)]
pub struct S5Codec(usize);

impl NewDrop for S5Codec {
    fn new() -> Self {
        Self(0)
    }

    fn drop(&mut self) {}
}

impl Encoder for S5Codec {
    type Item = Bytes;
    type Error = CodecError;

    fn encode(&mut self, line: Self::Item, buf: &mut BytesMut) -> Result<(), Self::Error> {
        let body_len = line.len();
        if body_len == 0 {
            return Ok(());
        }

        if self.0 == 1 && 0 == unsafe {*line.get_unchecked(1)} {
            self.0 += 1;
        }

        buf.reserve(body_len);
        buf.put(line);
        Ok(())
    }
}

impl Decoder for S5Codec {
    type Item = Bytes;
    type Error = CodecError;

    fn decode(&mut self, buf: &mut BytesMut) -> Result<Option<Self::Item>, Self::Error> {
        if buf.len() < 3 {
            return Ok(None);
        }

        // println!("self.0 = {}, buf = {:?}", self.0, buf.to_vec());
        match self.0 {
            0 => {
                // +----+----------+----------+
                // |VER | NMETHODS | METHODS  |
                // +----+----------+----------+
                // | 1  |    1     | 1 to 255 |
                // +----+----------+----------+
                let nmethods = unsafe { *buf.get_unchecked(1) as usize };
                let len = 2 + nmethods;
                if len <= buf.len() {
                    self.0 += 1;
                    return Ok(Some(buf.split_to(len).freeze()));
                } else {
                    return Ok(None);
                }
            }
            1 => {
                // +----+------+----------+------+----------+
                // |VER | ULEN |  UNAME   | PLEN |  PASSWD  |
                // +----+------+----------+------+----------+
                // | 1  |  1   | 1 to 255 |  1   | 1 to 255 |
                // +----+------+----------+------+----------+
                let ulen = unsafe { *buf.get_unchecked(1) as usize };
                let plen = if ulen > 1 {
                    unsafe { *buf.get_unchecked(2 + ulen) as usize }
                } else {
                    0
                };
                let len = if plen == 0 { 2 + ulen } else { 3 + ulen + plen };
                if len <= buf.len() {
                    self.0 += 1;
                    return Ok(Some(buf.split_to(len).freeze()));
                } else {
                    return Ok(None);
                }
            }
            2 => {
                // +----+-----+-------+------+----------+----------+
                // |VER | CMD |  RSV  | ATYP | DST.ADDR | DST.PORT |
                // +----+-----+-------+------+----------+----------+
                // | 1  |  1  | X'00' |  1   | Variable |    2     |
                // +----+-----+-------+------+----------+----------+
                let atyp = unsafe { *buf.get_unchecked(3) as usize };
                let addr_len = match atyp {
                    1 => 4,
                    3 => unsafe { *buf.get_unchecked(4) as usize + 1 },
                    4 => 16,
                    _ => {
                        assert!(false);
                        0
                    }
                };
                let len = 4 + addr_len + 2;
                if len <= buf.len() {
                    self.0 += 1;
                    return Ok(Some(buf.split_to(len).freeze()));
                } else {
                    return Ok(None);
                }
            }
            _ => Ok(Some(buf.split_to(buf.len()).freeze())),
        }
    }
}

// --
/*
#[derive(Copy, Clone, Debug)]
pub struct CodecWithRemote;

impl NewDrop for CodecWithRemote {
    fn new() -> Self {
        Self
    }

    fn drop(&mut self) {}
}

impl Encoder for CodecWithRemote {
    type Item = Bytes;
    type Error = CodecError;

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

impl Decoder for CodecWithRemote {
    type Item = Bytes;
    type Error = CodecError;

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
            // println!("b = {:?}, buf = {:?}", b, b.to_vec());
            Some(b.freeze())
        } else {
            None
        })
    }
}
*/
// --
/// A simple `Codec` implementation that just ships bytes around.
#[derive(Copy, Clone, Debug, Eq, PartialEq, Ord, PartialOrd, Hash)]
pub struct BytesCodec(());

impl BytesCodec {
    /// Creates a new `BytesCodec` for shipping around raw bytes.
    pub fn new() -> BytesCodec { BytesCodec(())  }
}

impl Decoder for BytesCodec {
    type Item = Bytes;
    type Error = CodecError;

    fn decode(&mut self, buf: &mut BytesMut) -> Result<Option<Self::Item>, Self::Error> {
        if buf.len() > 0 {
            let len = buf.len();
            Ok(Some(buf.split_to(len).freeze()))
        } else {
            Ok(None)
        }
    }
}

impl Encoder for BytesCodec {
    type Item = Bytes;
    type Error = CodecError;

    fn encode(&mut self, data: Bytes, buf: &mut BytesMut) -> Result<(), Self::Error> {
        buf.reserve(data.len());
        buf.put(data);
        Ok(())
    }
}
