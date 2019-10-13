use tokio::{prelude::*, net::TcpListener};

use tokio_codec::Decoder;

use std::env;

use bee::RecordCodec;

fn main() -> Result<(), Box<dyn std::error::Error>> {
    // Create the TCP listener we'll accept connections on.
    let addr = env::args().nth(1).unwrap_or("127.0.0.1:6142".to_string());
    let addr = addr.parse()?;

    let socket = TcpListener::bind(&addr)?;
    println!("Listening on: {}", addr);

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

            let (wt, rd) = RecordCodec::new().framed(stream).split();
            let connection = rd
                .map(|x| {
                    println!("recv: {:?}", x);
                    x
                })
                .forward(wt);

            let connection = connection.then(move |_| {
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
