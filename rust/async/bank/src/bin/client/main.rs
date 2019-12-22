// -- main.rs --

extern crate async_std;
// extern crate bytes;
// extern crate futures;
// extern crate futures_codec;
#[macro_use]
extern crate elog;
// extern crate codec;

// --

pub mod terminal;

// --

fn main() {
    init_ring_logger!(log::Level::Info, String::new(), 1024 * 1024 * 3, 3);
    terminal::run("127.0.0.1:1188".to_string());
}