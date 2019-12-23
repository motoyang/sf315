// -- adapter.rs --

use {
    async_std::{
        net::{TcpListener, ToSocketAddrs},
        prelude::*,
        task,
    },
    bank::{Dog, DogServant, Govement, GovementServant, Pusher, PusherServant},
    log::info,
    servant::{Adapter, Oid, ServantRegister, ServantResult},
};

// --

struct Receiver;

impl Pusher for Receiver {
    fn f1(&self, count: i32) {
        dbg!(count);
    }
    fn f2(&self) {
        dbg!("f2 called");
    }
    fn f3(&mut self, s: String) {
        dbg!(s);
    }

}

struct GovementEntry{
    _premier: String
}

impl Govement for GovementEntry {
    fn export(&self) -> Vec<Oid> {
        ServantRegister::instance().export()
    }
}

#[derive(Clone)]
struct Somedog {
    age: u32,
    name: String,
    wawa: String,
}

impl Dog for Somedog {
    fn speak(&self, count: i32) -> String {
        let mut s = String::new();
        for _ in 0..count {
            s.push_str(&self.wawa);
            s.push(',');
        }
        s
    }

    fn owner(&self) -> Oid {
        std::thread::sleep(std::time::Duration::from_secs(8));
        Oid::new("Tom".to_string(), "Person".to_string())
    }

    fn age(&mut self, i: u32) -> u32 {
        self.age += i;
        self.age
    }
}

// --

pub fn run(remote_addr: impl ToSocketAddrs) -> ServantResult<()> {
    let register = ServantRegister::instance();
    register.set_root_servant(Box::new(GovementServant::new("Chin@".to_string(),
        GovementEntry {_premier: "Mr. Lee".to_string()}
    )));
    register.add(Box::new(DogServant::new(
        "dog1".to_string(),
        Somedog {
            age: 1,
            name: "lg1".to_string(),
            wawa: "woo...".to_string(),
        },
    )));
    register.add(Box::new(PusherServant::new(
        "receiver".to_string(),
        Receiver,
    )));

    let r = task::block_on(accept_on(remote_addr));
    info!("run result: {:?}", r);

    Ok(())
}

async fn accept_on(addr: impl ToSocketAddrs) -> std::io::Result<()> {
    let listener = TcpListener::bind(addr).await?;
    let mut incoming = listener.incoming();
    while let Some(stream) = incoming.next().await {
        let stream = stream?;
        info!("Accepting from: {}", stream.peer_addr()?);

        let adapter = Adapter::new();
        let _handle = task::spawn(adapter.run(stream));
    }
    Ok(())
}
