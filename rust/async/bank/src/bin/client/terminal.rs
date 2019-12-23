// -- terminal.rs --

use {
    async_std::{prelude::*, stream, task},
    bank::{DogProxy, GovementProxy},
    futures::future::join_all,
    log::{error, info},
    servant::{PushMessage, Terminal},
    std::time::Duration,
};

// --

pub fn run(addr: String) {
    let mut terminal = Terminal::new(2);
    let terminal_handle = task::spawn(terminal.clone().run(addr));

    task::block_on(async {
        let mut i = 0_usize;
        let mut interval = stream::interval(Duration::from_secs(1)).take(3);
        while let Some(_) = interval.next().await {
            i += 1;
            let msg = format!("hello, #{}", i);
            let req = PushMessage::Hello { msg: msg.clone() };
            if let Err(e) = terminal.report(req).await {
                error!("{}", e.to_string());
            }

            let mut gov = GovementProxy::new(&terminal);
            match gov.export().await {
                Err(e) => error!("{}", e.to_string()),
                Ok(v) => println!("{}:{}, {:?}", file!(), line!(), &v),
            }

            let mut dog = terminal.proxy("dog1".to_string(), DogProxy::new);
            let r = dog.speak(2_i32).await;
            match r {
                Err(e) => error!("{}", e.to_string()),
                Ok(v) => println!("{}:{}, {:?}", file!(), line!(), &v),
            }
            let r = dog.owner().await;
            match r {
                Err(e) => error!("{}", e.to_string()),
                Ok(v) => println!("{}:{}, {:?}", file!(), line!(), &v),
            }
            let r = dog.age(2).await;
            match r {
                Err(e) => error!("{}", e.to_string()),
                Ok(v) => println!("{}:{}, {:?}", file!(), line!(), &v),
            }
            let r = dog.age(3).await;
            match r {
                Err(e) => error!("{}", e.to_string()),
                Ok(v) => println!("{}:{}, {:?}", file!(), line!(), &v),
            }
        }
        task::sleep(Duration::from_secs(1)).await;
        terminal.clean().await;
    });

    let v = vec![terminal_handle];
    let r = task::block_on(join_all(v));
    info!("run result: {:?}", r);
}