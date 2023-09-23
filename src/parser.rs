use std::fs::File;
use std::io::Read;
use crate::shared::{EnumFileType, StructXMLFileInfo, get_file_extension_by_file_type};
use crate::{log, write_log, write_log_no_time};

pub(crate) struct Parser {
    file_structure: StructXMLFileInfo,
    whole_file_data: Vec<u8>,
    file_size: u32,
    is_ascii: bool,
    first_time_is_ascii: bool,
}

impl Parser {
    pub fn new(file_structure: StructXMLFileInfo) -> Self {
        Self {
            file_structure,
            whole_file_data: Vec::new(),
            file_size: 0,
            is_ascii: false,
            first_time_is_ascii: true,
        }
    }
    pub fn load(&mut self) -> bool {
        let mut file = match File::open(self.get_file_name()) {
            Ok(data) => { data }
            Err(_) => {
                write_log!("ERROR: Can't open file '{}'.\n", self.get_file_name());
                return false
            }
        };

        write_log!("Reading file '{}'... ", self.get_file_name());

        self.file_size = match file.metadata() {
            Ok(data) => { data.len() as u32 }
            Err(_) => {
                write_log_no_time!("FAILED: Unable to get metadata from file.\n");
                write_log!("\n");
                return false
            }
        };

        if self.file_size < 1 {
            write_log_no_time!("FAILED: Empty File.\n");
            write_log!("\n");
            return false
        }

        self.whole_file_data = Vec::new();
        let read_bytes = match file.read_to_end(&mut self.whole_file_data) {
            Ok(data) => { data as u32 }
            Err(_) => {
                write_log_no_time!("FAILED: Unable to read file.\n");
                write_log!("\n");
                return false;
            }
        };

        if read_bytes != self.file_size {
            write_log_no_time!("FAILED: Mismatch read length bytes from file.\n");
            write_log!("\n");
            return false;
        }

        if ((self.get_file_type() == &EnumFileType::CsvFile || self.file_is_ascii()) && self.file_size < 3) || (!self.file_is_ascii() && self.file_size < 20) {
            write_log_no_time!("FAILED: File size is too small. Are you sure is a '{}' file?\n", get_file_extension_by_file_type(self.get_file_type()));
            write_log!("\n");
            return false;
        }

        if self.get_file_type() == &EnumFileType::CsvFile || self.file_is_ascii() {
            write_log_no_time!("DONE.\n");
            write_log!("Parsing file... ");
        }

        if !self.check_structure() {
             return false;
        }

        write_log_no_time!("DONE.\n");

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
    fn get_file_type(&self) -> &EnumFileType {
        &self.file_structure.file_type
    }
    fn file_is_ascii(&mut self) -> bool {
        if self.first_time_is_ascii {
            self.is_ascii = false;

            let mut binary = false;
            for &byte in &self.whole_file_data {
                if byte == b'\n' || byte == b'\r' {
                    continue
                }

                if byte == 0 || (byte < 32 && byte != 9 && byte != 10 && byte != 13) {
                    binary = true;
                    break;
                }
            }

            if binary {
                self.is_ascii = false;
            }
            else {
                self.is_ascii = true;
            }

        }

        self.is_ascii
    }
    fn check_structure(&self) -> bool {
        if self.get_file_type() == &EnumFileType::CsvFile || self.is_ascii {
            // todo: implement read csv files
        }
        else {
            let header = &self.whole_file_data[0..4];
            println!("Header: {}", String::from_utf8_lossy(header));

            let header_size: u32 = 20;

            let _total_records = &self.whole_file_data[4..8];
            let total_records = u32::from_le_bytes([_total_records[0], _total_records[1], _total_records[2], _total_records[3]]);
            println!("Total Records: {}", total_records);

            let _total_fields = &self.whole_file_data[8..12];
            let total_fields = u32::from_le_bytes([_total_fields[0], _total_fields[1], _total_fields[2], _total_fields[3]]);
            println!("Total Fields: {}", total_fields);

            let _record_size = &self.whole_file_data[12..16];
            let record_size = u32::from_le_bytes([_record_size[0], _record_size[1], _record_size[2], _record_size[3]]);
            println!("Record Size: {}", record_size);

            let _string_size = &self.whole_file_data[16..20];
            let string_size = u32::from_le_bytes([_string_size[0], _string_size[1], _string_size[2], _string_size[3]]);
            println!("String Size: {}", string_size);

            let data_bytes = self.file_size - header_size - string_size;
            let string_bytes = self.file_size - header_size - data_bytes;

            println!("databytes '{}' == (total_records * record_size) '{}'", data_bytes, (total_records * record_size));
            assert_eq!(data_bytes, total_records * record_size);

            println!("string_bytes '{}' == string_size '{}'", string_bytes, string_size);
            assert_eq!(string_bytes, string_size);

            println!("Ok, header match file.");
        }

        true
    }
}