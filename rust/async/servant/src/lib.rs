// -- libr.rs --

#[macro_use]
extern crate lazy_static;
extern crate async_std;
extern crate bincode;
extern crate futures;
extern crate futures_codec;
extern crate serde;
extern crate servant_macro;

// --

#[cfg(feature = "invoke")]
pub use servant_macro::invoke_interface;
#[cfg(feature = "notify")]
pub use servant_macro::notify_interface;
#[cfg(feature = "query")]
pub use servant_macro::query_interface;
#[cfg(feature = "report")]
pub use servant_macro::report_interface;

mod servant;

#[cfg(any(feature = "adapter", feature = "terminal"))]
mod drop_guard;

#[cfg(feature = "adapter")]
mod accept;
#[cfg(feature = "adapter")]
mod adapter;

#[cfg(feature = "terminal")]
mod terminal;

#[cfg(feature = "default_query")]
mod export;

// --

pub use servant::{
    NotifyServant, Oid, ReportServant, Servant, ServantError, ServantRegister, ServantResult,
};

#[cfg(feature = "adapter")]
pub use {
    accept::accept_on,
    adapter::{Adapter, AdapterRegister},
};

#[cfg(feature = "terminal")]
pub use terminal::Terminal;

#[cfg(feature = "default_query")]
pub use export::ExportProxy;
