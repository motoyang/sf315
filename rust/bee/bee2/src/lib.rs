// -- lib.rs --

#[macro_use]
extern crate lazy_static;

/*
use bytes::{Buf, BufMut, BytesMut, IntoBuf};
use futures::stream::Stream;
use tokio::{
    net::{TcpListener, TcpStream},
    prelude::*,
    runtime::current_thread,
};
use tokio_codec::{Decoder, Encoder};
*/

pub mod scropedlogger;
pub mod codec;
pub mod app;

// pub use codec::record::RecordCodec;

// use codec::record::RecordCodec;

// --
/*
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
        let len = if buf_len > head_len {
            b.get_u16_le() as usize
        } else {
            return Ok(None);
        };

        Ok(if len + head_len <= buf_len {
            let b = buf.split_to(head_len + len).split_off(head_len);
            Some(b.freeze())
        } else {
            None
        })
    }
}
*/
/*
#[derive(Copy, Clone)]
pub struct RecordCodec2;

impl RecordCodec2 {
    pub fn new() -> RecordCodec2 {
        RecordCodec2
    }
}

#[derive(Debug, Default)]
pub struct CodecError<T>(T)
where
    T: std::fmt::Debug;

impl<T> std::fmt::Display for CodecError<T>
where
    T: std::fmt::Debug,
{
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        write!(f, "ErrorA!")
    }
}

impl<T> std::error::Error for CodecError<T>
where
    T: std::fmt::Debug,
{
    fn description(&self) -> &str {
        "Description for ErrorA"
    }
    fn source(&self) -> Option<&(dyn std::error::Error + 'static)> {
        None
    }
}

impl<T> std::convert::From<std::io::Error> for CodecError<T>
where
    T: std::fmt::Debug + Default,
{
    fn from(_e: std::io::Error) -> CodecError<T> {
        Default::default()
    }
}

impl<T> std::convert::From<futures::sync::mpsc::SendError<Vec<u8>>> for CodecError<T>
where
    T: std::fmt::Debug + Default,
{
    fn from(_e: futures::sync::mpsc::SendError<Vec<u8>>) -> CodecError<T> {
        Default::default()
    }
}

impl<T> std::convert::From<()> for CodecError<T>
where
    T: std::fmt::Debug + Default,
{
    fn from(_e: ()) -> CodecError<T> {
        Default::default()
    }
}

impl Encoder for RecordCodec2 {
    type Item = Vec<u8>;
    type Error = CodecError<Vec<u8>>;

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

impl Decoder for RecordCodec2 {
    type Item = Vec<u8>;
    type Error = CodecError<Vec<u8>>;

    // Find the next line in buf!
    fn decode(&mut self, buf: &mut BytesMut) -> Result<Option<Self::Item>, Self::Error> {
        let head_len = std::mem::size_of::<u16>();
        let buf_len = buf.len();
        let mut b = buf.clone().freeze().into_buf();
        let len = if buf_len > head_len {
            b.get_u16_le() as usize
        } else {
            return Ok(None);
        };

        Ok(if len + head_len <= buf_len {
            let b = buf.split_to(head_len + len).split_off(head_len);
            Some(b.to_vec())
        } else {
            None
        })
    }
}
*/
// --
/*
pub trait Servant<C>: std::marker::Sync + std::marker::Send
where
    C: Encoder<Item = Vec<u8>, Error = std::io::Error>
        + Decoder<Item = Bytes, Error = std::io::Error>,
{
    fn codec() -> C;
    fn addr(&self) -> std::net::SocketAddr;

    fn take_over(
        &self,
        _: Result<<C as Decoder>::Item, <C as Decoder>::Error>,
    ) -> Result<Option<<C as Encoder>::Item>, <C as Encoder>::Error> {
        Ok(None)
    }

    fn notify(
        &self,
    ) -> Box<
        dyn Stream<Item = Option<<C as Encoder>::Item>, Error = <C as Encoder>::Error>
            + std::marker::Send,
    > {
        Box::new(futures::stream::empty())
    }
}

pub fn start_client<T, C>(c: T)
where
    T: 'static + Servant<C>,
    C: 'static
        + Decoder<Item = Bytes, Error = std::io::Error>
        + Encoder<Item = Vec<u8>, Error = std::io::Error>
        + std::marker::Send,
{
    let client = TcpStream::connect(&c.addr())
        .and_then(move |stream| {
            println!("created stream");

            let codec = T::codec();
            let (wt, rd) = codec.framed(stream).split();

            let notify = c.notify();
            let connection = rd
                .then(move |msg| c.take_over(msg))
                .select(notify)
                .map(|v| v.unwrap_or(Vec::<u8>::new()))
                .forward(wt)
                .then(|_| {
                    println!("Connection closed.");
                    Ok(())
                });
            connection
        })
        .map_err(|err| {
            println!("connection error = {:?}", err);
        });

    // Following snippets come here...

    // println!("About to create the stream and write to it...");
    // tokio::run(client);
    current_thread::run(client);
    // println!("Stream has been created and written to.");

    // true
}

pub fn start_server<C, T>(servant: T) -> Result<(), Box<dyn std::error::Error>>
where
    T: 'static + Servant<C> + std::marker::Copy,
    C: 'static
        + Decoder<Item = Bytes, Error = std::io::Error>
        + Encoder<Item = Vec<u8>, Error = std::io::Error>
        + std::marker::Send
        + std::marker::Copy
        + std::marker::Sync,
{
    let socket = TcpListener::bind(&servant.addr())?;
    println!("Listening on: {}", &servant.addr());

    // The server task asynchronously iterates over and processes each incoming
    // connection.
    let srv = socket
        .incoming()
        .map_err(|e| {
            println!("failed to accept socket; error = {:?}", e);
            e
        })
        .for_each(move |stream| {
            let addr = stream.peer_addr()?;
            println!("New Connection: {}", addr);

            let (wt, rd) = T::codec().framed(stream).split();
            let notify = servant.notify();
            let connection = rd
                .then(move |x| servant.take_over(x))
                .select(notify)
                .map(|v| v.unwrap_or(Vec::<u8>::new()))
                .forward(wt)
                .then(move |_| {
                    println!("Connection {} closed.", addr);
                    Ok(())
                });

            // Spawn a task to process the connection
            tokio::spawn(connection);
            Ok(())
        })
        .map_err(|err| println!("error occurred: {:?}", err));

    // execute server
    tokio::run(srv);
    Ok(())
}
*/
// --
/*
pub trait Worker<C>: std::marker::Sync + std::marker::Send
where
    C: Encoder<Item = Vec<u8>, Error = CodecError<Vec<u8>>>
        + Decoder<Item = Vec<u8>, Error = CodecError<Vec<u8>>>,
{
    fn codec() -> C;
    fn addr(&self) -> std::net::SocketAddr;
    fn channel() -> (
        futures::sync::mpsc::Sender<Vec<u8>>,
        futures::sync::mpsc::Receiver<Vec<u8>>,
    );
    fn channel2() -> (u8, u8);

    fn take_over(
        &self,
        _: Result<<C as Encoder>::Item, <C as Encoder>::Error>,
    ) -> Result<<C as Encoder>::Item, <C as Encoder>::Error> {
        Ok(Vec::<u8>::with_capacity(0))
    }

    fn notify(
        &self,
    ) -> Box<
        dyn Stream<Item = <C as Encoder>::Item, Error = <C as Encoder>::Error> + std::marker::Send,
    > {
        Box::new(futures::stream::empty())
    }
}

pub fn start_worker<C, T>(servant: T) -> Result<(), Box<dyn std::error::Error>>
where
    T: 'static + Worker<C> + std::marker::Copy,
    C: 'static
        + Decoder<Item = Vec<u8>, Error = CodecError<Vec<u8>>>
        + Encoder<Item = Vec<u8>, Error = CodecError<Vec<u8>>>
        + std::marker::Send
        + std::marker::Copy
        + std::marker::Sync,
{
    let socket = TcpListener::bind(&servant.addr())?;
    println!("Listening on: {}", &servant.addr());

    // The server task asynchronously iterates over and processes each incoming
    // connection.
    let srv = socket
        .incoming()
        .map_err(|e| {
            println!("failed to accept socket; error = {:?}", e);
            e
        })
        .for_each(move |stream| {
            let addr = stream.peer_addr()?;
            println!("New Connection: {}", addr);

            let (wt, rd) = T::codec().framed(stream).split();
            let (tx, rx) = futures::sync::mpsc::channel::<Vec<u8>>(10);
            let read_socket = rd.forward(tx).map_err(|_| ()).map(|_| ());
            let write_socket = rx
                .map_err(|_| {
                    let e: CodecError<Vec<u8>> = Default::default();
                    e
                })
                .forward(wt)
                .map_err(|_| ())
                .map(|_| ());

            let connection = write_socket.select(read_socket).then(|_| Ok(()));

            // Spawn a task to process the connection
            tokio::spawn(connection);
            Ok(())
        })
        .map_err(|err| println!("error occurred: {:?}", err));

    // execute server
    tokio::run(srv);
    Ok(())
}
*/