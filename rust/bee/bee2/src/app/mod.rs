// -- mod.rs --

use bytes::Bytes;
use futures::stream::Stream;
use tokio_codec::{Decoder, Encoder};

// --

pub trait NewDrop {
    fn new() -> Self;
    fn drop(&mut self);
}

pub trait Application: std::marker::Sync + std::marker::Send {
    type Codec: Encoder<Item = Vec<u8>, Error = std::io::Error>
        + Decoder<Item = Bytes, Error = std::io::Error>
        + NewDrop
        + Copy
        + Send;

    fn addr(&self) -> std::net::SocketAddr;

    fn take_over(
        &self,
        _: Result<<Self::Codec as Decoder>::Item, <Self::Codec as Decoder>::Error>,
    ) -> Result<Option<<Self::Codec as Encoder>::Item>, <Self::Codec as Encoder>::Error> {
        Ok(None)
    }

    fn notify(
        &self,
    ) -> Box<
        dyn Stream<
                Item = Option<<Self::Codec as Encoder>::Item>,
                Error = <Self::Codec as Encoder>::Error,
            > + std::marker::Send,
    > {
        Box::new(futures::stream::empty())
    }
}

// --

mod single;
pub use single::single_thread_client;
pub use single::thread_pool_server;
