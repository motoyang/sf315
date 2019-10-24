// -- sink.rs --

use {
    ring_channel::RingReceiver,
    std::{
        fs::{File, OpenOptions},
        io::{self, Write},
        thread::{self, JoinHandle},
    },
};

// --

pub fn spawn(
    mut rx: RingReceiver<String>,
    mut name: String,
    limit: u64,
    roll: usize,
) -> JoinHandle<()> {
    thread::spawn(move || {
        if name.is_empty() {
            name = std::env::current_exe()
                .expect("current_exe() error")
                .file_name()
                .expect("file_name() error")
                .to_str()
                .expect("to_str() error")
                .to_string();
        }
        let mut f = FileSink::new(name, limit, roll);

        while let Ok(msg) = rx.recv() {
            f.write(msg.as_bytes()).expect("write to file error");
        }
    })
}

// --

struct FileSink {
    name: String,
    limit: u64,
    roll: usize,
    current_roll: usize,
    file: File,
}

impl FileSink {
    fn new(name: String, limit: u64, roll: usize) -> Self {
        let current_roll = 0_usize;
        Self {
            name: name.clone(),
            limit: if limit == 0 { u64::max_value() } else { limit },
            roll: if roll == 0 { usize::max_value() } else { roll },
            current_roll,
            file: FileSink::create_file(&FileSink::roll_name(name, current_roll)),
        }
    }
    fn roll_name(mut name: String, roll: usize) -> String {
        name.push_str(&format!(".{}.txt", roll));
        name
    }
    fn create_file(name: &str) -> File {
        OpenOptions::new()
            .write(true)
            .create(true)
            .truncate(true)
            .open(&name)
            .expect("open sinkfile error.")
    }
    fn roll_file(&mut self) {
        self.flush().expect("flush file error");

        self.current_roll += 1;
        self.current_roll %= self.roll;
        let n = FileSink::roll_name(self.name.clone(), self.current_roll);
        self.file = FileSink::create_file(&n);
    }
    fn check(&mut self) {
        let md = self.file.metadata().expect("get metadata error");
        if md.len() > self.limit {
            self.roll_file();
        }
    }
}

impl Drop for FileSink {
    fn drop(&mut self) {
        self.flush().expect("flush file error");
    }
}

impl Write for FileSink {
    fn write(&mut self, msg: &[u8]) -> io::Result<usize> {
        self.check();
        self.file.write(msg)
    }
    fn flush(&mut self) -> io::Result<()> {
        self.file.flush().expect("flush file error");
        Ok(())
    }
}
