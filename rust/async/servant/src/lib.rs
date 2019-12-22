// -- libr.rs --

extern crate async_std;
extern crate bincode;
extern crate bytes;
extern crate futures;
extern crate futures_codec;
extern crate serde;

// --

pub mod drop_guard;
mod servant;
mod terminal;
mod adapter;

// --

pub use servant::{ServantResult, ServantError, Oid, ServantRegister, Servant, PushMessage};
pub use terminal::{Terminal, OutOfBandProxy};
pub use adapter::{Adapter, OutOfBandEntry, OutOfBandServant};


