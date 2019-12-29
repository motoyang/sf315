// -- lib.rs --

extern crate async_std;
extern crate bincode;
extern crate futures;
extern crate serde;
extern crate servant;
extern crate servant_macro;

// --

use servant::{NotifyServant, Oid, ReportServant, Servant, ServantResult, Terminal};

// --

#[servant_macro::invoke_interface]
pub trait Dog: Clone {
    fn speak(&self, count: i32) -> String;
    fn owner(&self) -> Oid;
    fn age(&mut self, i: u32) -> u32;
}

#[servant_macro::query_interface]
pub trait Govement {
    fn export_servants(&self) -> Vec<Oid>;
    fn export_report_servants(&self) -> Vec<Oid>;
}

#[servant_macro::report_interface]
pub trait Pusher {
    fn f1(&self, count: i32);
    fn f2(&self);
    fn f3(&mut self, s: String);
}

#[servant_macro::notify_interface]
pub trait StockNews: Clone {
    fn f1(&self, count: i32);
    fn f2(&self, msg: String);
    fn f3(&mut self, count: usize, f: f64, b: Option<bool>, s: Vec<String>);
}
