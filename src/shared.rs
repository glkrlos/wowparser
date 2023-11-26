struct cShared;

impl cShared {
    fn instance() -> &'static cShared {
        static INSTANCE: cShared = cShared;
        &INSTANCE
    }
}

lazy_static! {
    static ref SHARED: &'static cShared = cShared::instance();
}