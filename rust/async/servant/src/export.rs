// -- export.rs --

use async_std::task;
use super::{
    adapter::AdapterRegister,
    servant::{Oid, ServantRegister},
};
use crate as servant;

// --

#[servant_macro::query_interface]
pub trait Export {
    fn export_servants(&self) -> Vec<Oid>;
    fn export_report_servants(&self) -> Vec<Oid>;
    fn shutdown(&self, passcode: usize);
}

// --

pub struct ExportEntry;

impl Export for ExportEntry {
    fn export_servants(&self) -> Vec<Oid> {
        ServantRegister::instance().export_servants()
    }
    fn export_report_servants(&self) -> Vec<Oid> {
        ServantRegister::instance().export_report_servants()
    }
    fn shutdown(&self, passcode: usize) {
        task::block_on(async {
            AdapterRegister::instance().clean(passcode).await;
        });
    }
}
