// -- record_codec.rs --

use {
    crate::utility::Length,
    bytes::{buf::ext::BufMutExt, Buf, BytesMut},
    futures_codec::{Decoder, Encoder},
    std::io::{Error, ErrorKind},
};

// --

#[derive(Default, Clone, Copy)]
pub struct RecordCodec<H, R>(std::marker::PhantomData<H>, std::marker::PhantomData<R>);

impl<H, R> Encoder for RecordCodec<H, R>
where
    H: Length,
    R: serde::Serialize,
{
    type Item = R;
    type Error = Error;

    fn encode(&mut self, src: Self::Item, dst: &mut BytesMut) -> Result<(), Self::Error> {
        let head_len = std::mem::size_of_val(&H::from_usize(Default::default()));
        let len = bincode::serialized_size(&src)
            .map_err(|e| Error::new(ErrorKind::Other, e.to_string()))? as usize;
        if len > H::max() {
            return Err(Error::new(
                ErrorKind::Other,
                format!("record is too long. length = {}", len),
            ));
        }
        dst.reserve(head_len + len);
        H::from_usize(len).put(dst);
        bincode::serialize_into(dst.writer(), &src)
            .map_err(|e| Error::new(ErrorKind::Other, e.to_string()))
    }
}

impl<H, R> Decoder for RecordCodec<H, R>
where
    H: Length,
    for<'de> R: serde::Deserialize<'de>,
{
    type Item = R;
    type Error = Error;

    fn decode(&mut self, src: &mut BytesMut) -> Result<Option<Self::Item>, Self::Error> {
        let head_len = std::mem::size_of_val(&H::from_usize(Default::default()));
        if src.len() < head_len {
            return Ok(None);
        }

        let len: usize = H::get(&src[..head_len]).to_usize();
        if src.len() - head_len >= len {
            src.advance(head_len);
            Ok(Some(
                bincode::deserialize_from(src.split_to(len).as_ref())
                    .map_err(|e| Error::new(ErrorKind::Other, e.to_string()))?,
            ))
        } else {
            Ok(None)
        }
    }
}

// --

#[cfg(test)]
mod tests {
    extern crate test_case;

    use super::*;
    use futures::{executor, io::Cursor, sink::SinkExt, TryStreamExt};
    use futures_codec::{FramedRead, FramedWrite};

    #[derive(Debug, Clone, Default, Eq, PartialEq, serde::Serialize, serde::Deserialize)]
    struct Person {
        name: String,
        age: u8,
        phones: Vec<String>,
    }
    #[derive(Debug, Clone, Default, Eq, PartialEq, serde::Serialize, serde::Deserialize)]
    struct Person2 {
        name: String,
        age: u8,
        phones: Vec<u8>,
    }

    // --

    #[test]
    fn t_record_u32_codec() {
        executor::block_on(async move {
            let mut buf = vec![];
            let cur = Cursor::new(&mut buf);
            let mut framed = FramedWrite::new(cur, RecordCodec::<u32, Person>::default());

            let msg = Person {
                name: "moto".to_string(),
                age: 18,
                phones: vec!["123".to_string(), "456".to_string()],
            };
            framed.send(msg.clone()).await.unwrap();
            println!("buf: {:?}", buf);

            let mut framed2 = FramedRead::new(buf.as_slice(), RecordCodec::<u32, Person>::default());
            let msg2 = framed2.try_next().await.unwrap().unwrap();
            println!("msg: {:?}", msg2);

            assert_eq!(msg, msg2);
        });
    }

    #[test]
    fn t_record_u64_codec() {
        executor::block_on(async move {
            let mut buf = vec![];
            let cur = Cursor::new(&mut buf);
            let mut framed = FramedWrite::new(cur, RecordCodec::<u64, Person2>::default());

            let msg = Person2 {
                name: "moto2".to_string(),
                age: 188,
                phones: vec![8; 32000],
            };
            framed.send(msg.clone()).await.unwrap();
            println!("buf: {:?}", buf);

            let mut framed2 = FramedRead::new(buf.as_slice(), RecordCodec::<u64, Person2>::default());
            let msg2 = framed2.try_next().await.unwrap().unwrap();
            println!("msg: {:?}", msg2);

            assert_eq!(msg, msg2);
        });
    }

}
