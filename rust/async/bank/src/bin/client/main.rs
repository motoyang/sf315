// -- main.rs --

extern crate async_std;
extern crate futures;
extern crate servant;
extern crate bank;
#[macro_use]
extern crate elog;

// --

pub mod terminal;

// --

fn main() {
    init_ring_logger!(log::Level::Info, String::new(), 1024 * 1024 * 3, 3);
    terminal::run("127.0.0.1:1188".to_string());
}