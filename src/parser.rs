use std::fs::File;
use std::io::Read;
use crate::shared::{EnumFileType, StructXMLFileInfo, get_file_extension_by_file_type};
use crate::{log, write_log, write_log_no_time};

pub(crate) struct Parser {
    file_structure: StructXMLFileInfo
}

impl Parser {
    pub fn new(file_structure: StructXMLFileInfo) -> Self {
        Self { file_structure }
    }
    pub fn load(&self) -> bool {
        let mut file = match File::open(self.get_file_name()) {
            Ok(data) => { data }
            Err(_) => {
                write_log!("ERROR: Can't open file '{}'.\n", self.get_file_name());
                return false
            }
        };

        write_log!("Reading file '{}'... ", self.get_file_name());

        let file_size = match file.metadata() {
            Ok(data) => { data.len() }
            Err(_) => {
                write_log_no_time!("FAILED: Unable to get metadata from file.\n");
                write_log!("\n");
                return false
            }
        };

        if file_size < 1 {
            write_log_no_time!("FAILED: Empty File.\n");
            write_log!("\n");
            return false
        }

        let mut whole_file_data = Vec::new();
        let read_bytes = match file.read_to_end(&mut whole_file_data) {
            Ok(data) => { data as u64 }
            Err(_) => {
                write_log_no_time!("FAILED: Unable to read file.\n");
                write_log!("\n");
                return false;
            }
        };

        if read_bytes != file_size {
            write_log_no_time!("FAILED: Mismatch read length bytes from file.\n");
            write_log!("\n");
            return false;
        }

        let is_ascii = self.file_is_ascii(&whole_file_data);

        if ((self.get_file_type() == &EnumFileType::CsvFile || is_ascii) && file_size < 3) || (!is_ascii && file_size < 20) {
            write_log_no_time!("FAILED: File size is too small. Are you sure is a '{}' file?\n", get_file_extension_by_file_type(self.get_file_type()));
            write_log!("\n");
            return false;
        }

        if self.get_file_type() == &EnumFileType::CsvFile || is_ascii {
            write_log_no_time!("DONE.\n");
            write_log!("Parsing file... ");
        }

        // if !self.check_structure() {
        //     return false;
        // }

        write_log_no_time!("DONE.\n");

        true
    }
    fn get_file_type(&self) -> &EnumFileType {
        &self.file_structure.file_type
    }
    fn file_is_ascii(&self, data: &[u8]) -> bool {
        for &byte in data {
            if byte == b'\n' || byte == b'\r' {
                continue
            }

            if byte == 0 || (byte < 32 && byte != 9 && byte != 10 && byte != 13) {
                return false;
            }
        }

        true
    }
    pub fn parse_file(&self){
        //print!("{}", self.get_file_name());
        // just for ignoring unused struct
        let _ = self.file_structure;
    }

    fn get_file_name(&self) -> &str {
        self.file_structure.file_name.as_str()
    }
}