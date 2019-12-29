// -- adapter.rs --

use {
    async_std::{
        net::{TcpListener, ToSocketAddrs},
        prelude::*,
        stream,
        task,
    },
    bank::{Dog, DogServant, Govement, GovementServant, Pusher, PusherReportServant, StockNewsSender},
    log::info,
    servant::{Adapter, Notifier, Oid, ServantRegister, ServantResult},
    std::{time::Duration, sync::{Arc, Mutex}},
};

// --

// #[derive(Clone)]
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

// #[derive(Clone)]
struct GovementEntry {
    _premier: String,
}

impl Govement for GovementEntry {
    fn export_servants(&self) -> Vec<Oid> {
        ServantRegister::instance().export_servants()
    }
    fn export_report_servants(&self) -> Vec<Oid> {
        ServantRegister::instance().export_report_servants()
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
        // std::thread::sleep(std::time::Duration::from_secs(8));
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
    let query = Arc::new(Mutex::new(GovementServant::new(
        "Chin@".to_string(),
        GovementEntry {
            _premier: "Mr. Lee".to_string(),
        },
    )));
    register.set_query_servant(query);

    register.add_servant(Arc::new(Mutex::new(DogServant::new(
        "dog1".to_string(),
        Somedog {
            age: 1,
            name: "lg1".to_string(),
            wawa: "woo...".to_string(),
        },
    ))));
    register.add_report_servant(Arc::new(Mutex::new(PusherReportServant::new(
        "receiver".to_string(),
        Receiver,
    ))));

    let notifier = Notifier::instance();
    let n2 = notifier.clone();
    task::spawn(n2.run());
    std::thread::sleep(Duration::from_millis(1000));

    let notifier_handle = task::spawn(notifier_run());

    let r = task::block_on(accept_on(remote_addr));
    info!("run result: {:?}", r);

    let r = task::block_on(notifier_handle);
    info!("notifier run result: {:?}", r);

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

    Notifier::instance().clean().await;
    Ok(())
}

async fn notifier_run() -> std::io::Result<()> {
    let n = Notifier::instance();
    let mut sender1 = StockNewsSender::new(n.clone());
    let mut sender2 = StockNewsSender::new(n.clone());

    let mut i = 0_usize;
    let mut interval = stream::interval(Duration::from_secs(1)).take(100);
    while let Some(_) = interval.next().await {
        i += 1;
        let msg = format!("notice, #{}", i);

        sender1.f1(i as i32).await.unwrap();
        dbg!(i);

        std::thread::sleep(Duration::from_millis(300));
        sender2.f2(msg.clone()).await.unwrap();
        dbg!(msg);
    }
    Ok(())
}
