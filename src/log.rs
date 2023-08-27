use std::fs::OpenOptions;
use std::io::Write;
use std::time::{SystemTime, UNIX_EPOCH, Duration};
use lazy_static::lazy_static;
use std::sync::Mutex;

static WOW_PARSER_LOG_OUTPUT: &str = "wowparser4.log";

pub struct CLog {
    log_file: Option<std::fs::File>,
}

impl CLog {
    fn new() -> Self {
        let log_file = OpenOptions::new()
            .write(true)
            .create(true)
            .truncate(true)
            .open(WOW_PARSER_LOG_OUTPUT)
            .ok();

        CLog { log_file }
    }

    pub fn write_log(&mut self, args: &str) {
        if let Some(file) = &mut self.log_file {
            let current_time = SystemTime::now().duration_since(UNIX_EPOCH).unwrap();
            let formatted_time = format_time(current_time);
            let formatted_log = format!("{} {}\n", formatted_time, args);
            file.write_all(formatted_log.as_bytes()).unwrap();
        }
    }

    pub fn write_log_no_time(&mut self, args: &str) {
        if let Some(file) = &mut self.log_file {
            let formatted_log = format!("{}\n", args);
            file.write_all(formatted_log.as_bytes()).unwrap();
        }
    }

    pub fn write_log_no_time_and_print(&mut self, args: &str) {
        println!("{}", args);
        self.write_log_no_time(args);
    }

    pub fn write_log_and_print(&mut self, args: &str) {
        println!("{}", args);
        self.write_log(args);
    }
}

fn format_time(duration: Duration) -> String {
    let seconds = duration.as_secs();
    let nanos = duration.subsec_nanos();
    let formatted_time = format!(
        "{:04}-{:02}-{:02} {:02}:{:02}:{:02}.{:09}",
        1970 + seconds / (365 * 24 * 3600),
        (seconds / (30 * 24 * 3600)) % 12 + 1,
        (seconds / (24 * 3600)) % 30 + 1,
        (seconds / 3600) % 24,
        (seconds / 60) % 60,
        seconds % 60,
        nanos
    );
    formatted_time
}

lazy_static! {
    pub static ref INSTANCE: Mutex<CLog> = Mutex::new(CLog::new());
}

pub fn instance() -> std::sync::MutexGuard<'static, CLog> {
    INSTANCE.lock().unwrap()
}