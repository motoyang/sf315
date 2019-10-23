// -- main.rs --

#[macro_use]
extern crate elog;

// --

fn main() {
    init_ring_logger!(log::Level::Trace, String::new(), 1024*32, 3);

    log::debug!("this is a debug {}", "message");
    log::error!("this is printed by default");

    if log::log_enabled!(log::Level::Info) {
        let x = 3 * 4; // expensive computation
        log::info!("the answer was: {}", x);
    }

    for i in 0..2000 {
        log::info!("this is {}", i);
    }

    example::test();
}

// --

mod example {
    pub fn test() {
        log::info!("from Example::test()");
    }
}

// --

#[cfg(test)]
mod tests {
    #[test]
    fn it_works() {
        super::logger::testcase_init();

        log::error!("This record will be captured by `cargo test`");

        assert_eq!(2, 1 + 12);
    }
}
