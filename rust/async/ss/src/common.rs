// -- common.rs --

use {
    bytes::{Buf, BufMut, Bytes, BytesMut},
};

// --

pub type BoxResult<T> = std::result::Result<T, Box<dyn std::error::Error + Send + Sync>>;

// --

pub fn extract(mut b: Bytes) -> (u64, Bytes) {
    let mut rb = b.split_to(std::mem::size_of::<u64>());
    (rb.get_u64(), b)
}

pub fn pack(id: u64, msg: Bytes) -> Bytes {
    let len = std::mem::size_of_val(&id) + msg.len();
    let mut b = BytesMut::with_capacity(len);
    b.put_u64(id);
    b.put_slice(&msg);
    b.freeze()
}
