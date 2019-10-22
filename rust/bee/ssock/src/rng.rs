use ring::rand::*;

// --

#[allow(unused)]
pub fn rand_gen<T>(rng: &dyn SecureRandom) -> T
where
    T: Sized + Copy,
{
    let len = std::mem::size_of::<T>();
    let mut r = Vec::<u8>::with_capacity(len);
    r.resize(len, 0);
    rng.fill(&mut r).unwrap();
    let ptr = r.as_mut_ptr() as *const u8 as *const T;
    unsafe { *ptr }
}

pub fn no_zero_rand_gen<T>(rng: &dyn SecureRandom) -> T
where
    T: Sized + Copy + PartialEq + Default,
{
    let mut value: T;
    let len = std::mem::size_of::<T>();
    let mut r = Vec::<u8>::with_capacity(len);
    r.resize(len, 0);
    loop {
        rng.fill(&mut r).unwrap();
        let ptr = r.as_mut_ptr() as *const u8 as *const T;
        value = unsafe { *ptr };
        if value != T::default() {
            return value;
        }
    }
}
