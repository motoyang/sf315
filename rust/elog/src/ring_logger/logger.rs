// -- logger.rs --

use chrono::Local;
use log::{Level, Log, Metadata, Record};

use super::sink::spawn;
use ring_channel::{ring_channel, RingSender};
use std::{fmt::Write as FmtWrite, num::NonZeroUsize, thread::JoinHandle};

// --

static mut SENDER: Option<RingSender<String>> = None;

// --

fn spawn_sink(name: String, limit: u64, roll: usize) -> JoinHandle<()> {
    unsafe {
        assert!(SENDER.is_none());
    }

    const CAPICITY: usize = 10;
    let (tx, rx) = ring_channel(NonZeroUsize::new(CAPICITY).unwrap());
    unsafe {
        SENDER = Some(tx);
    }
    spawn(rx, name, limit, roll)
}

// --

pub struct RingLogger {
    sink_handle: Option<JoinHandle<()>>,
}

impl RingLogger {
    pub fn new(level: Level, name: String, limit: u64, roll: usize) -> RingLogger {
        let sink_handle = Some(spawn_sink(name, limit, roll));

        let logger = Logger { level };
        log::set_boxed_logger(Box::new(logger)).unwrap();
        log::set_max_level(level.to_level_filter());
        log::trace!("start of ring-logger");

        RingLogger { sink_handle }
    }
}

impl Drop for RingLogger {
    fn drop(&mut self) {
        log::info!("end of ring-logger");

        unsafe {
            // 关闭sender，sink_spawn线程也就自动退出了。
            SENDER = None;
        }

        // 等待sink_spawn线程退出
        self.sink_handle.take().map(JoinHandle::join);
    }
}

// --

struct Logger {
    level: Level,
}

impl Log for Logger {
    fn enabled(&self, metadata: &Metadata) -> bool {
        metadata.level() <= self.level
    }

    fn log(&self, record: &Record) {
        if self.enabled(record.metadata()) {
            let level_string = { record.level().to_string() };
            let target = if record.target().len() > 0 {
                record.target()
            } else {
                record.module_path().unwrap_or_default()
            };
            let mut msg = String::new();
            writeln!(
                &mut msg,
                "{} {:<5} [{}:{}:{}] {}",
                Local::now().format("%Y-%m-%d %H:%M:%S.%6f"),
                level_string,
                target,
                record.file().unwrap_or("<unknown file>"),
                record.line().unwrap_or(0),
                record.args()
            )
            .expect("writeln error");

            if let Some(tx) = unsafe { SENDER.as_mut() } {
                tx.send(msg).expect("send error");
            } else {
                panic!("panic: SENDER has been set to None.");
            }
        }
    }

    fn flush(&self) {}
}
