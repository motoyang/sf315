// -- client.rs --

extern crate ring;

use ring::{error::Unspecified, rand::*, aead::*};

// --

struct Sequence([u8; NONCE_LEN]);

impl Sequence {
    fn new() -> Self {
        let mut seq: Self = Sequence([0; NONCE_LEN]);
        // let rng = SystemRandom::new();
        // rng.fill(&mut seq.0).unwrap();
        seq
    }
}
impl NonceSequence for Sequence {
    fn advance(&mut self) -> Result<Nonce, Unspecified> {
        let ptr = self.0.as_mut_ptr();
        let p_u32 = ptr as *mut u32;
        let p_u64: *mut u64;
        unsafe {
            p_u64 = ptr.offset(std::mem::size_of::<u32>() as isize) as *mut u64;
            *p_u64 += 1;
            *p_u32 += 3;
        }
        Nonce::try_assume_unique_for_key(&self.0)
    }
}

fn main() {
    let s = "hello";
    let vs = s.as_ref() as &[u8];
    let vs2 = s.as_bytes();
    let vvs = &[vs, vs2][..];
    let vs3 = s.bytes().collect::<Vec<u8>>();
    println!("{:?}", vs);
    println!("{:?}", vs2);
    println!("{:?}", vs3);
    println!("{:?}", vvs);


    let lengths = [4, 1, 2, 8];

    let rng = SystemRandom::new();
    for len in lengths.iter() {
        let mut buf = vec![0; *len];
        let r = rng.fill(&mut buf);
        println!("r = {:?} buf = {:?}", r, buf);
    }

    let key = CHACHA20_POLY1305.key_len();
    let mut key = Vec::<u8>::with_capacity(key);
    key.resize(key.capacity(), 2);
    assert_eq!(key.len(), CHACHA20_POLY1305.key_len());

    let ubk = UnboundKey::new(&CHACHA20_POLY1305, &key).unwrap();
    let mut sealed_key = SealingKey::new(ubk, Sequence::new());
    println!("key = {:?}", sealed_key);

    let mut plain: Vec::<u8> = "abcdefg".bytes().collect();
    println!("p = {:?}", plain);
    let r = sealed_key.seal_in_place_append_tag(Aad::empty(), &mut plain);
    println!("c = {:?}, len = {}", plain, plain.len());

    let ubk2 = UnboundKey::new(&CHACHA20_POLY1305, &key).unwrap();
    let mut open_key = OpeningKey::new(ubk2, Sequence::new());
    let mut p = plain.as_mut_slice();
    let r = open_key.open_in_place(Aad::empty(), &mut p).unwrap();
    // println!("p = {:?}", &p);
    println!("r = {:?}", r);

}
