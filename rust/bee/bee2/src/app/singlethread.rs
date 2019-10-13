// -- single.rs --

use futures::stream::Stream;
use log::{error, info};
use tokio::{
    net::{TcpStream},
    prelude::*,
    runtime::current_thread,
};
use tokio_codec::{Decoder};
use super::{Application, NewDrop};

// --

pub fn single_thread_client<T>(app: T)
where
    T: 'static + Application,
{
    let client = TcpStream::connect(&app.addr())
        .and_then(move |stream| {
            info!("created stream");

            let mut codec = T::Codec::new();
            let (wt, rd) = codec.framed(stream).split();

            let notify = app.notify();
            let connection = rd
                .then(move |msg| app.take_over(msg))
                .select(notify)
                .map(|v| v.unwrap_or(Vec::<u8>::new()))
                .forward(wt)
                .then(move |_| {
                    info!("Connection closed.");
                    codec.drop();
                    Ok(())
                });
            connection
        })
        .map_err(|err| {
            error!("connection error = {:?}", err);
        });

    current_thread::run(client);
}
