// -- terminal.rs --

use {
    async_std::{prelude::*, stream, task},
    bank::{DogProxy, PusherReportProxy, StockNews, StockNewsReceiver},
    futures::future::join_all,
    log::info,
    servant::Terminal,
    std::time::Duration,
};

// --

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
    let receiver = Box::new(StockNewsReceiver::new(News));
    let terminal = Terminal::new(2, Some(receiver));
    // let mut terminal = Terminal::new(2, None);
    let t2 = terminal.clone();
    let terminal_handle = task::spawn(async {
        let r = t2.run(addr).await;
        info!("terminal run result: {:?}", r);
    });
    std::thread::sleep(Duration::from_secs(1));

    task::block_on(async {
        let mut i = 0_usize;
        let mut interval = stream::interval(Duration::from_millis(110)).take(90);
        while let Some(_) = interval.next().await {
            i += 1;
            let msg = format!("hello, #{}", i);

            let mut pusher = terminal.proxy("receiver", PusherReportProxy::new);
            let r = pusher.f1(i as i32).await;
            assert!(dbg!(r).is_ok());
            let r = pusher.f3(msg).await;
            assert!(dbg!(r).is_ok());

            let mut gov = servant::ExportProxy::new(&terminal);
            let r = gov.export_servants().await;
            assert!(dbg!(r).is_ok());
            let r = gov.export_report_servants().await;
            assert!(dbg!(r).is_ok());

            let mut dog = terminal.proxy("dog0", DogProxy::new);
            let r = dog.speak(2).await;
            assert!(dbg!(r).is_err());

            let mut dog = terminal.proxy("dog1", DogProxy::new);
            let r = dog.speak(2_i32).await;
            assert!(dbg!(r).is_ok());
            let r = dog.owner().await;
            assert!(dbg!(r).is_ok());
            let r = dog.age(2).await;
            assert!(dbg!(r).is_ok());
            let r = dog.age(3).await;
            assert!(dbg!(r).is_ok());
        }
        let mut gov = servant::ExportProxy::new(&terminal);
        let r = gov.shutdown(238).await;
        assert!(dbg!(r).is_ok());

        task::sleep(Duration::from_secs(1)).await;
        terminal.clean().await;
    });

    let v = vec![terminal_handle];
    let r = task::block_on(join_all(v));
    info!("run result: {:?}", r);
}
