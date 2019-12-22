// --

extern crate async_std;
#[macro_use]
extern crate elog;

// --

mod adapter;

// --

fn main() {
    init_ring_logger!(log::Level::Info, String::new(), 1024*1024*3, 3);
    adapter::run("0.0.0.0:1188").unwrap();
}
