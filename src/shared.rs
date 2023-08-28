use lazy_static::lazy_static;
use std::sync::{Mutex, Arc};

#[macro_export]
macro_rules! getch {
    () => {
            println!("Press enter to continue...");
            let mut buffer = [0u8; 1];
            let stdin = io::stdin();

            stdin.lock().read(&mut buffer).unwrap();
    };
}

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