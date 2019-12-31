// -- accept.rs --

use {
    super::adapter::{Adapter, AdapterRegister},
    async_std::{
        net::{TcpListener, TcpStream, ToSocketAddrs},
        prelude::*,
        task,
    },
    futures::{channel::mpsc::unbounded, pin_mut, select, FutureExt as _},
    log::info,
};

// --

pub async fn accept_on(addr: impl ToSocketAddrs) -> std::io::Result<()> {
    #[derive(Debug)]
    enum SelectedValue {
        RxNone,
        IncomingNone,
        Incoming(TcpStream),
    };

    let (tx, rx) = unbounded();
    AdapterRegister::instance().set_accept(tx).await;

    let listener = TcpListener::bind(addr).await?;
    let incoming = listener.incoming();

    pin_mut!(incoming, rx);
    loop {
        let value = select! {
            connection_stream = incoming.next().fuse() => match connection_stream {
                Some(stream) => SelectedValue::Incoming(stream?),
                None => SelectedValue::IncomingNone,
            },
            from_rx = rx.next().fuse() => match from_rx {
                Some(record) => unreachable!(),
                None => SelectedValue::RxNone,
            },
        };
        match value {
            SelectedValue::Incoming(stream) => {
                info!("Accepting from: {}", stream.peer_addr()?);
                let adapter = Adapter::new();
                let _handle = task::spawn(adapter.run(stream));
            }
            _ => {
                info!("accept loop break due to {:?}", value);
                break;
            }
        }
    }

    Ok(())
}
