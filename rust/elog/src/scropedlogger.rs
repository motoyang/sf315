// -- logger.rs --

use chrono::Local;

// --

#[macro_export]
macro_rules! init_scroped_logger {
    () => {
        #[allow(unused)]
        let scroped_logger = elog::ScropedLogger::new();
    };
}

// --

pub struct ScropedLogger;

impl ScropedLogger {
    #[allow(dead_code)]
    pub fn new() -> ScropedLogger {
        env_logger::builder()
            .format(|buf, record| {
                use std::io::Write;
                // let ts = buf.timestamp_nanos();
                writeln!(
                    buf,
                    "[{} {:<5} {} {}:{}] {}",
                    // ts,
                    Local::now().format("%Y-%m-%d %H:%M:%S.%6f"),
                    buf.default_styled_level(record.level()),
                    record.module_path().unwrap_or("<unknown module>"),
                    record.file().unwrap_or("<unknown file>"),
                    record.line().unwrap_or(0),
                    record.args()
                )
            })
            .init();

        log::info!("start of scroped logger");
        ScropedLogger {}
    }
}

impl Drop for ScropedLogger {
    fn drop(&mut self) {
        log::info!("end of scroped logger");
    }
}
