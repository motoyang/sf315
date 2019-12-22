// -- lib.rs --

extern crate async_std;
extern crate bincode;
extern crate serde;
extern crate futures;
// extern crate bytes;
// extern crate futures_codec;

extern crate servant;
extern crate servant_interface;

// --

use servant::{Oid, Servant, ServantResult, Terminal};

// std::include!("person.rs");

#[servant_interface::interface]
pub trait Dog: Clone {
    fn speak(&self, count: i32) -> String;
    fn owner(&self) -> Oid;
    fn age(&mut self, i: u32) -> u32;
}

#[servant_interface::interface]
pub trait Govement {
    fn export(&self) -> Vec<Oid>;
}
