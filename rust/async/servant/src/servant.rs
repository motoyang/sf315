// -- servant.rs --

use {
    serde::{Deserialize, Serialize},
    std::{collections::HashMap, error::Error},
    std::sync::{Arc, Mutex}
};

// --

#[derive(Debug, Clone, Default, Serialize, Deserialize)]
pub struct ServantError {
    desc: String,
}

impl Error for ServantError {}

impl<T: Into<String>> std::convert::From<T> for ServantError {
    fn from(e: T) -> Self {
        Self { desc: e.into() }
    }
}

impl std::fmt::Display for ServantError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "Servant({})", self.desc)
    }
}

pub type ServantResult<T> = Result<T, ServantError>;

// --

#[derive(Clone, Debug, Eq, PartialEq, Hash, serde::Serialize, serde::Deserialize)]
pub struct Oid {
    name: String,
    category: String,
}

impl Oid {
    pub fn new(name: String, category: String) -> Self {
        Self { name, category }
    }
    pub fn name(&self) -> &str {
        &self.name
    }
    pub fn category(&self) -> &str {
        &self.category
    }
}

impl std::fmt::Display for Oid {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "Oid({}: {})", self.name, self.category)
    }
}

// --

lazy_static! {
    static ref REGISTER: ServantRegister = ServantRegister(Mutex::new(_ServantRegister {
        servants: HashMap::new(),
        report_servants: HashMap::new(),
        query: None
    }));
}

pub type ServantEntry = Arc<Mutex<dyn Servant + Send>>;
pub type ReportServantEntry = Arc<Mutex<dyn ReportServant + Send>>;
pub struct _ServantRegister {
    servants: HashMap<Oid, ServantEntry>,
    report_servants: HashMap<Oid, ReportServantEntry>,
    query: Option<ServantEntry>,
}

pub struct ServantRegister(Mutex<_ServantRegister>);
impl ServantRegister {
    pub fn instance() -> &'static Self {
        &REGISTER
    }
    pub fn set_query_servant(&self, query: ServantEntry) {
        let mut g = self.0.lock().unwrap();
        g.query.replace(query);
    }
    pub fn query_servant(&self) -> ServantEntry {
        let g = self.0.lock().unwrap();
        g.query.as_ref().unwrap().clone()
    }
    pub fn find_servant(&self, oid: &Oid) -> Option<ServantEntry> {
        let g = self.0.lock().unwrap();
        g.servants.get(&oid).map(|s| s.clone())
    }
    pub fn find_report_servant(&self, oid: &Oid) -> Option<ReportServantEntry> {
        let g = self.0.lock().unwrap();
        g.report_servants.get(&oid).map(|s| s.clone())
    }
    pub fn add_servant(&self, obj: ServantEntry) {
        let oid = {
            let g = obj.lock().unwrap();
            Oid::new(String::from(g.name()), String::from(g.category()))
        };
        let mut g = self.0.lock().unwrap();
        g.servants.insert(oid, obj);
    }
    pub fn add_report_servant(&self, entry: ReportServantEntry) {
        let oid = {
            let g = entry.lock().unwrap();
            Oid::new(String::from(g.name()), String::from(g.category()))
        };
        let mut g = self.0.lock().unwrap();
        g.report_servants.insert(oid, entry);
    }
    pub fn export_servants(&self) -> Vec<Oid> {
        let g = self.0.lock().unwrap();
        g.servants.keys().map(|x| x.clone()).collect()
    }
    pub fn export_report_servants(&self) -> Vec<Oid> {
        let g = self.0.lock().unwrap();
        g.report_servants.keys().map(|x| x.clone()).collect()
    }
}

// --
/*
pub trait ServantClone {
    fn clone_box(&self) -> Box<dyn NotifyServant + Send>;
}

impl<T> ServantClone for T
where
    T: 'static + NotifyServant + Clone + Send,
{
    fn clone_box(&self) -> Box<dyn NotifyServant + Send> {
        Box::new(self.clone())
    }
}

impl Clone for Box<dyn NotifyServant + Send> {
    fn clone(&self) -> Box<dyn NotifyServant + Send> {
        self.clone_box()
    }
}

impl std::fmt::Debug for Box<dyn NotifyServant + Send> {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "(Box<dyn NotifyServant>)")
    }
}
*/
pub trait NotifyServant {
    fn serve(&mut self, req: Vec<u8>);
}

pub trait Servant {
    fn name(&self) -> &str;
    fn category(&self) -> &'static str;
    fn serve(&mut self, req: Vec<u8>) -> ServantResult<Vec<u8>>;
}

pub trait ReportServant {
    fn name(&self) -> &str;
    fn category(&self) -> &'static str;
    fn serve(&mut self, req: Vec<u8>);
}

// --

#[derive(Debug, Clone, Serialize, Deserialize)]
pub enum Record {
    Notice {
        id: usize,
        msg: Vec<u8>
    },
    Report {
        id: usize,
        oid: Oid,
        msg: Vec<u8>,
    },
    Request {
        id: usize,
        oid: Option<Oid>,
        req: Vec<u8>,
    },
    Response {
        id: usize,
        oid: Option<Oid>,
        ret: Vec<u8>,
    },
}
