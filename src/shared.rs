use lazy_static::lazy_static;
use std::sync::{Mutex, Arc};

pub struct Sshared;

impl Sshared {
    pub fn show(&self) {
        println!("Shared");
    }
}

lazy_static! {
    static ref INSTANCE: Arc<Mutex<Sshared>> = Arc::new(Mutex::new(Sshared));
}

pub fn instance() -> std::sync::MutexGuard<'static, Sshared> {
    INSTANCE.lock().unwrap()
}