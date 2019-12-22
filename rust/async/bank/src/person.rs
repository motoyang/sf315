// --

use servant::{Oid, Servant, ServantResult, Terminal};

// --

pub trait Dog: Clone {
    fn speak(&mut self, count: i32) -> String;
    fn owner(&mut self) -> Oid;
}

#[derive(Clone)]
pub struct DogServant<S> {
    name: String,
    entity: S,
}

impl<S> DogServant<S> {
    pub fn new(name: String, entity: S) -> Self {
        Self { name, entity }
    }
}

impl<S> Servant for DogServant<S>
where
    S: Dog + 'static,
{
    fn name(&self) -> &str {
        &self.name
    }
    fn category(&self) -> &'static str {
        "Dog"
    }
    fn serve(&mut self, req: Vec<u8>) -> ServantResult<Vec<u8>> {
        let req: DogRequest = bincode::deserialize(&req).unwrap();
        let reps = match req {
            DogRequest::Speak { count } => bincode::serialize(&self.entity.speak(count)),
            DogRequest::Owner {} => bincode::serialize(&self.entity.owner()),
        }
        .unwrap();
        Ok(reps)
    }
}

#[derive(Debug, serde::Serialize, serde::Deserialize)]
enum DogRequest {
    Speak { count: i32 },
    Owner {},
}

#[allow(unused)]
#[derive(Clone, Debug)]
pub struct DogProxy(Oid, Terminal);

impl DogProxy {
    pub fn new(name: String, t: Terminal) -> Self {
        let oid = Oid::new(name, "Dog".to_string());
        Self(oid, t.clone())
    }

    #[allow(unused)]
    pub async fn speak(&mut self, count: i32) -> ServantResult<String> {
        let request = DogRequest::Speak { count };
        let response = self
            .1
            .invoke(Some(self.0.clone()), bincode::serialize(&request).unwrap())
            .await?;
        Ok(bincode::deserialize(&response).unwrap())
    }

    #[allow(unused)]
    pub async fn owner(&mut self) -> ServantResult<Oid> {
        let request = DogRequest::Owner {};
        let response = self
            .1
            .invoke(Some(self.0.clone()), bincode::serialize(&request).unwrap())
            .await?;
        Ok(bincode::deserialize(&response).unwrap())
    }
}
/*
// --

pub trait Person: Clone {
    fn name(&self, context: tarpc::context::Context) -> String;
    fn age(&self, context: tarpc::context::Context) -> i32;
    fn say(&self, context: tarpc::context::Context, count: i32, msg: String) -> Vec<String>;
}

// --

#[derive(Clone)]
pub struct PersonServant<S> {
    name: String,
    entity: S,
}

impl<S> PersonServant<S> {
    pub fn new(name: String, entity: S) -> Self {
        Self { name, entity }
    }
}

impl<S> Servant for PersonServant<S>
where
    S: Person + 'static,
{
    fn name(&self) -> &str {
        &self.name
    }
    fn category(&self) -> &'static str {
        "Person"
    }
    fn serve(&mut self, ctx: tarpc::context::Context, req: Vec<u8>) -> ServantResult<Vec<u8>> {
        let req: PersonRequest = bincode::deserialize(&req).unwrap();
        let resp = match req {
            PersonRequest::Name {} => bincode::serialize(&Person::name(&self.entity, ctx)),
            PersonRequest::Age {} => bincode::serialize(&Person::age(&self.entity, ctx)),
            PersonRequest::Say { count, msg } => {
                bincode::serialize(&Person::say(&self.entity, ctx, count, msg))
            }
        }
        .unwrap();
        Ok(resp)
    }
}

#[derive(Debug, serde::Serialize, serde::Deserialize)]
enum PersonRequest {
    Name {},
    Age {},
    Say { count: i32, msg: String },
}

// --

#[allow(unused)]
#[derive(Clone, Debug)]
pub struct PersonProxy(Oid, std::sync::Arc<std::sync::Mutex<Terminal>>);

impl PersonProxy {
    pub fn new(name: String, t: std::sync::Arc<std::sync::Mutex<Terminal>>) -> Self {
        let oid = Oid::new(name, "Person".to_string());
        Self(oid, t.clone())
    }

    async fn dispatch(
        &mut self,
        ctx: tarpc::context::Context,
        buf: Vec<u8>,
    ) -> ServantResult<Vec<u8>> {
        let oid = self.0.clone();
        let mut t = self.1.lock().unwrap();
        t.dispatch(ctx, oid, buf).await
    }

    #[allow(unused)]
    pub async fn name(&mut self, ctx: tarpc::context::Context) -> ServantResult<String> {
        let request = PersonRequest::Name {};
        let response = self
            .dispatch(ctx, bincode::serialize(&request).unwrap())
            .await?;
        Ok(bincode::deserialize(&response).unwrap())
    }

    #[allow(unused)]
    pub async fn age(&mut self, ctx: tarpc::context::Context) -> ServantResult<i32> {
        let request = PersonRequest::Age {};
        let response = self
            .dispatch(ctx, bincode::serialize(&request).unwrap())
            .await?;
        Ok(bincode::deserialize(&response).unwrap())
    }

    #[allow(unused)]
    pub async fn say(
        &mut self,
        ctx: tarpc::context::Context,
        count: i32,
        msg: String,
    ) -> ServantResult<Vec<String>> {
        let request = PersonRequest::Say { count, msg };
        let response = self
            .dispatch(ctx, bincode::serialize(&request).unwrap())
            .await?;
        Ok(bincode::deserialize(&response).unwrap())
    }
}
*/
