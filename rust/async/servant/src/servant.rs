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
    pub fn name(&self) -> String {
        self.name.clone()
    }
    pub fn category(&self) -> String {
        self.category.clone()
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
        query: None
    }));
}

pub struct _ServantRegister {
    servants: HashMap<Oid, Arc<Mutex<dyn Servant + Send>>>,
    query: Option<Arc<Mutex<dyn Servant + Send>>>,
}

pub struct ServantRegister(Mutex<_ServantRegister>);
impl ServantRegister {
    pub fn instance() -> &'static Self {
        &REGISTER
    }
    pub fn set_query_servant(&self, query: Arc<Mutex<dyn Servant + Send>>) {
        let mut g = self.0.lock().unwrap();
        g.query.replace(query);
    }
    pub fn query_servant(&self) -> Arc<Mutex<dyn Servant>> {
        let g = self.0.lock().unwrap();
        g.query.as_ref().unwrap().clone()
    }
    pub fn find(&self, oid: &Oid) -> Option<Arc<Mutex<dyn Servant>>> {
        let g = self.0.lock().unwrap();
        if let Some(s) = g.servants.get(&oid) {
            Some(s.clone())
        } else {
            None
        }
    }
    pub fn add(&self, obj: Arc<Mutex<dyn Servant + Send>>) {
        let oid = {
            let g = obj.lock().unwrap();
            Oid::new(String::from(g.name()), String::from(g.category()))
        };
        let mut g = self.0.lock().unwrap();
        g.servants.insert(oid, obj);
    }
    pub fn export(&self) -> Vec<Oid> {
        let g = self.0.lock().unwrap();
        g.servants.keys().map(|x| x.clone()).collect()
    }
}

// --

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
        // (*self.
        write!(f, "(Box<dyn Servant>)")
    }
}

pub trait NotifyServant: ServantClone {
    fn serve(&mut self, req: Vec<u8>) -> ServantResult<Vec<u8>>;
}

pub trait Servant {
    fn name(&self) -> &str;
    fn category(&self) -> &'static str;
    fn serve(&mut self, req: Vec<u8>) -> ServantResult<Vec<u8>>;
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
    Invoke {
        id: usize,
        oid: Option<Oid>,
        req: Vec<u8>,
    },
    Return {
        id: usize,
        oid: Option<Oid>,
        ret: Vec<u8>,
    },
}
