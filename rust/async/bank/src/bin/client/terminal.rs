// -- terminal.rs --

use {
    async_std::{prelude::*, stream, task},
    bank::{DogProxy, GovementProxy, PusherReportProxy, StockNews, StockNewsReceiver},
    futures::future::join_all,
    log::info,
    servant::Terminal,
    std::time::Duration,
};

// --

#[derive(Clone)]
struct News;
impl StockNews for News {
    fn f1(&self, count: i32) {
        dbg!(count);
    }
    fn f2(&self, msg: String) {
        dbg!(msg);
    }
    fn f3(&mut self, count: usize, f: f64, b: Option<bool>, s: Vec<String>) {
        dbg!(count, f, b, s);
    }
}

// --

pub fn run(addr: String) {
    let receiver = Box::new(StockNewsReceiver::new("stock receiver".to_string(), News));
    let mut terminal = Terminal::new(2, receiver);
    let t2 = terminal.clone();
    let terminal_handle = task::spawn(async {
        let r = t2.run(addr).await;
        info!("terminal run result: {:?}", r);
    });

    task::block_on(async {
        let mut i = 0_usize;
        let mut interval = stream::interval(Duration::from_secs(5)).take(3);
        while let Some(_) = interval.next().await {
            i += 1;
            let msg = format!("hello, #{}", i);

            let mut pusher = terminal.proxy("receiver".to_string(), PusherReportProxy::new);
            let r = pusher.f1(i as i32).await;
            assert!(dbg!(r).is_ok());
            let r = pusher.f3(msg).await;
            assert!(dbg!(r).is_ok());

            let mut gov = GovementProxy::new(&terminal);
            let r = gov.export_servants().await;
            assert!(dbg!(r).is_ok());
            let r = gov.export_report_servants().await;
            assert!(dbg!(r).is_ok());

            let mut dog = terminal.proxy("dog0".to_string(), DogProxy::new);
            let r = dog.speak(2).await;
            assert!(dbg!(r).is_err());

            let mut dog = terminal.proxy("dog1".to_string(), DogProxy::new);
            let r = dog.speak(2_i32).await;
            assert!(dbg!(r).is_ok());
            let r = dog.owner().await;
            assert!(dbg!(r).is_ok());
            let r = dog.age(2).await;
            assert!(dbg!(r).is_ok());
            let r = dog.age(3).await;
            assert!(dbg!(r).is_ok());
        }
        task::sleep(Duration::from_secs(1)).await;
        terminal.clean().await;
    });

    let v = vec![terminal_handle];
    let r = task::block_on(join_all(v));
    info!("run result: {:?}", r);
}
