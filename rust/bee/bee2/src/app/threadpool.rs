// -- threadpool.rs --

use futures::stream::Stream;
use log::{error, info};
use tokio::{
    net::{TcpListener, TcpStream},
    prelude::*,
};
use tokio_codec::{Decoder};
use super::{Application, NewDrop};

// --

pub fn thread_pool_client<T>(app: T)
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

    tokio::run(client);
}

pub fn thread_pool_server<T>(app: T) -> Result<(), Box<dyn std::error::Error>>
where
    T: 'static + Application + Copy,
{
    let socket = TcpListener::bind(&app.addr())?;
    info!("Listening on: {}", &app.addr());

    let srv = socket
        .incoming()
        .map_err(|e| {
            error!("failed to accept socket; error = {:?}", e);
            e
        })
        .for_each(move |stream| {
            let addr = stream.peer_addr()?;
            info!("New Connection: {}", addr);

            let mut codec = T::Codec::new();
            let (wt, rd) = codec.framed(stream).split();
            let notify = app.notify();
            let connection = rd
                .then(move |x| app.take_over(x))
                .select(notify)
                .map(|v| v.unwrap_or(Vec::<u8>::new()))
                .forward(wt)
                .then(move |_| {
                    info!("Connection {} closed.", addr);
                    codec.drop();
                    Ok(())
                });

            // Spawn a task to process the connection
            tokio::spawn(connection);
            Ok(())
        })
        .map_err(|err| error!("error occurred: {:?}", err));

    // execute server
    tokio::run(srv);
    Ok(())
}
