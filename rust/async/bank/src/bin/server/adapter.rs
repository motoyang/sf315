// -- adapter.rs --

use {
    async_std::{
        net::{ ToSocketAddrs},
        prelude::*,
        stream, task,
    },
    bank::{Dog, DogServant, Pusher, PusherReportServant, StockNewsNotifier},
    log::info,
    servant::{ Oid, ServantRegister, ServantResult, accept_on},
    std::{
        sync::{Arc, Mutex},
        time::Duration,
    },
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

    let _notifier_handle = task::spawn(notifier_run());

    let r = task::block_on(accept_on(remote_addr));
    info!("run result: {:?}", r);

    // let r = task::block_on(notifier_handle);
    // info!("notifier run result: {:?}", r);

    Ok(())
}

#[allow(unused)]
async fn notifier_run() -> std::io::Result<()> {
    let sender1 = StockNewsNotifier::instance();
    let mut b = false;
    let mut i = 0_usize;
    let mut interval = stream::interval(Duration::from_millis(100)).take(10000000);
    while let Some(_) = interval.next().await {
        i += 1;
        let msg = format!("notice, #{}", i);

        match i % 3 {
            0 => sender1.f1(i as i32).await,
            1 => sender1.f2(msg.clone()).await,
            2 => {
                b = !b;
                let s = vec!["hello".to_string(); i % 37];
                sender1
                    .f3(
                        i,
                        i as f64 * 2.0,
                        if i % 2 == 0 { Some(b) } else { None },
                        s,
                    )
                    .await
                    ;
            }
            _ => unreachable!(),
        }
    }
    Ok(())
}
