use std::fs::OpenOptions;
use std::io::Write;
use chrono::{Local, Datelike, Timelike};
use lazy_static::lazy_static;
use std::sync::Mutex;

static WOW_PARSER_LOG_OUTPUT: &str = "wowparser4.log";

#[macro_export]
macro_rules! write_log_and_print {
    ($($arg:tt)*) => {
        let formated = format!($($arg)*);
        log::instance().write_log(&formated, true);
        print!("{}", formated);
    };
}

#[macro_export]
macro_rules! write_log_no_time_and_print {
    ($($arg:tt)*) => {
        let formated = format!($($arg)*);
        log::instance().write_log(&formated, false);
        print!("{}", formated);
    };
}

#[macro_export]
macro_rules! write_log {
    ($($arg:tt)*) => {
        let formated = format!($($arg)*);
        log::instance().write_log(&formated, true);
    };
}

#[macro_export]
macro_rules! write_log_no_time {
    ($($arg:tt)*) => {
        let formated = format!($($arg)*);
        log::instance().write_log(&formated, false);
    };
}

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

        Self { log_file }
    }

    pub fn write_log(&mut self, args: &str, time: bool) {
        if let Some(file) = &mut self.log_file {
            let formatted_log;
            if time {
                let current_time = Local::now();
                let year = current_time.year();
                let month = current_time.month();
                let day = current_time.day();
                let hour = current_time.hour();
                let minute = current_time.minute();
                let second = current_time.second();
                let nanosecond = current_time.nanosecond() / 1_000;
                formatted_log = format!("{:04}-{:02}-{:02} {:02}:{:02}:{:02}.{:06} {}", year, month, day, hour, minute, second, nanosecond, args);
            } else {
                formatted_log = format!("{}", args);
            }

            file.write_all(formatted_log.as_bytes()).unwrap();
        }
    }
}

lazy_static! {
    pub static ref INSTANCE: Mutex<CLog> = Mutex::new(CLog::new());
}

pub fn instance() -> std::sync::MutexGuard<'static, CLog> {
    INSTANCE.lock().unwrap()
}