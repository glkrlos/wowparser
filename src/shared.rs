use lazy_static::lazy_static;

pub struct Sshared;

impl Sshared {
    pub fn instance() -> &'static Sshared {
        static INSTANCE: Sshared = Sshared;
        &INSTANCE
    }
    pub fn show(&self) {
        println!("Shared");
    }
}

lazy_static! {
    static ref SHARED: &'static Sshared = Sshared::instance();
}