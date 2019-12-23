// -- servant.rs --

use {
    serde::{Deserialize, Serialize},
    std::{collections::HashMap, error::Error},
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

pub struct ServantRegister {
    m: HashMap<Oid, Box<dyn Servant>>,
    out_of_band: Option<Box<dyn Servant>>,
}

impl ServantRegister {
    pub fn instance() -> &'static mut ServantRegister {
        static mut REGISTER: Option<ServantRegister> = None;

        unsafe {
            REGISTER.get_or_insert_with(|| ServantRegister {
                m: HashMap::new(),
                out_of_band: None,
            })
        }
    }
    pub fn set_root_servant(&mut self, root: Box<dyn Servant>) {
        self.out_of_band.replace(root);
    }
    pub fn root_servant(&mut self) -> Option<&mut Box<dyn Servant>> {
        self.out_of_band.as_mut()
    }
    pub fn find(&mut self, oid: &Oid) -> Option<&mut Box<dyn Servant>> {
        self.m.get_mut(&oid)
    }

    pub fn add(&mut self, obj: Box<dyn Servant>) {
        let oid = Oid::new(String::from(obj.name()), String::from(obj.category()));
        self.m.insert(oid, obj);
    }

    pub fn export(&self) -> Vec<Oid> {
        self.m.keys().map(|x| x.clone()).collect()
    }
}

// --

pub trait Servant {
    fn name(&self) -> &str;
    fn category(&self) -> &'static str;
    fn serve(&mut self, req: Vec<u8>) -> ServantResult<Vec<u8>>;
}

// --
/*
#[derive(Debug, Serialize, Deserialize)]
pub enum PushMessage {
    Hello { msg: String },
}
*/
#[derive(Debug, Serialize, Deserialize)]
pub enum Record {
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
