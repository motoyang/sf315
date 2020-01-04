// -- lib.rs --

extern crate async_std;
extern crate bincode;
extern crate futures;
extern crate serde;
extern crate servant;
// extern crate servant_macro;

// --

#[servant::invoke_interface]
pub trait Dog: Clone {
    fn speak(&self, count: i32) -> String;
    fn owner(&self) -> servant::Oid;
    fn age(&mut self, i: u32) -> u32;
}
/*
#[servant::query_interface]
pub trait Govement {
    fn export_servants(&self) -> Vec<Oid>;
    fn export_report_servants(&self) -> Vec<Oid>;
}
*/
#[servant::report_interface]
pub trait Pusher {
    fn f1(&self, count: i32);
    fn f2(&self);
    fn f3(&mut self, s: String);
}

#[servant::notify_interface]
pub trait StockNews {
    fn f1(&self, count: i32);
    fn f2(&self, msg: String);
    fn f3(&mut self, count: usize, f: f64, b: Option<bool>, s: Vec<String>);
}
