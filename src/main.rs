mod staticdata;
mod shared;
mod log;

#[allow(unused_imports)]
use shared::EnumFieldTypes;
#[allow(unused_imports)]
use shared::EnumFileType;

use std::io::{self, Read};

fn print_header() {
    write_log_and_print!("WoWParser Version {} ({}) in Pre-Alpha Development State for {} {} (Revision: {})", staticdata::version(), staticdata::codename(), staticdata::os(), staticdata::arch(), staticdata::revision());
    write_log_and_print!("Hash: {}\tDate: {}", staticdata::hash(), staticdata::date());
    write_log_and_print!("");
    write_log_and_print!("Tool to Parse World of Warcraft files (DBC DB2 ADB WDB).");
    write_log_and_print!("Copyright(c) 2023 Carlos Ramzuel - Huamantla, Tlaxcala, Mexico.");
    write_log_and_print!("");
}

fn pass1_loadconfig() {
}

fn pass2_printfilestolog() {
}

fn pass3_checkheadersanddataconsistency() {
}

fn print_end() {
    write_log_and_print!("-----> Finished");
}

fn main() {
    shared::instance().show();

    print_header();

    pass1_loadconfig();

    pass2_printfilestolog();

    pass3_checkheadersanddataconsistency();

    print_end();

    getch!();
}
