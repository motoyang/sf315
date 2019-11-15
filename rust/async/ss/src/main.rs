extern crate futures;
extern crate futures_codec;
extern crate async_std;
extern crate codec;
extern crate bytes;
#[macro_use] extern crate elog;

// --

mod common;
mod drop_guard;
mod local;
mod remote;

// --

use {
    clap::{App, Arg, ArgGroup},
    std::net::ToSocketAddrs,
};

// --

fn main() {
    init_ring_logger!(log::Level::Trace, String::new(), 1024*1024*3, 3);

    let matches = App::new("s-sock")
        .version("1.0")
        .author("moto <kbknapp@gmail.com>")
        .about("Another shadow-sock build with rust.")
        .arg(Arg::from_usage(
            "[local-addr] -l --local-addr = [ADDR_LOCAL] 'Sets local server address'")
            .requires("remote-addr")
        )
        .arg(Arg::from_usage(
            "[remote-addr] -r --remote-addr = [ADDR_REMOTE] 'Sets remote server address'",
        ))
        .group(ArgGroup::with_name("address")
            .args(&["local-addr", "remote-addr"])
            .multiple(true)
            .required(true)
        )
        .get_matches();

    let local_addr = if let Some(addr) = matches.value_of("local-addr") {
        if let Ok(mut addr) = addr.to_socket_addrs() {
            addr.next()
        } else {
            println!("local-addr is invalid.");
            return;
        }
    } else {
        None
    };

    let remote_addr = if let Some(addr) = matches.value_of("remote-addr") {
        if let Ok(mut addr) = addr.to_socket_addrs() {
            addr.next()
        } else {
            println!("remote-addr is invalid.");
            return;
        }
    } else {
        None
    };

    if matches.is_present("local-addr") {
        local::run(local_addr.unwrap(), remote_addr.unwrap()).unwrap();
    }

    if matches.is_present("remote-addr") && !matches.is_present("local-addr") {
        remote::run(remote_addr.unwrap()).unwrap();
    }
}