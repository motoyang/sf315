// -- mod.rs --

mod sink;
mod logger;

// --

#[macro_export]
macro_rules! init_ring_logger {
    ($a:expr, $b:expr, $c:expr, $d:expr) => {
        #[allow(unused)]
        let ring_logger = elog::RingLogger::new($a, $b, $c, $d);
    };
}

// --

pub use logger::RingLogger;
