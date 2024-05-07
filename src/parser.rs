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
    first_time_get_header: bool,
    header_offset: u32,
    header_magic: String
}

impl Parser {
    pub fn new(file_structure: StructXMLFileInfo) -> Self {
        Self {
            file_structure,
            whole_file_data: Vec::new(),
            file_size: 0,
            is_ascii: false,
            first_time_is_ascii: true,
            first_time_get_header: true,
            header_offset: 0,
            header_magic: String::new()
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
    fn header_get_uint(&mut self) -> u32 {
        let mut raw_data: [u8; 4] = [0; 4];
        let current_offset = self.header_get_offset(4) as usize;
        raw_data.copy_from_slice(&self.whole_file_data[current_offset..self.header_offset as usize]);

        u32::from_le_bytes(raw_data)
    }
    fn header_get_char(&mut self) -> char {
        let current_offset = self.header_get_offset(1) as usize;
        char::from(self.whole_file_data[current_offset])
    }
    fn header_get_offset(&mut self, size: u32) -> u32 {
        if (self.header_offset + size) <= self.file_size {
            self.header_offset += size;

            self.header_offset - size
        }
        else {
            0
        }
    }
    fn get_file_type_by_header(&mut self) -> EnumFileType {
        if self.first_time_get_header {
            let mut header_magic = String::new();
            header_magic.push(self.header_get_char());
            header_magic.push(self.header_get_char());
            header_magic.push(self.header_get_char());
            header_magic.push(self.header_get_char());
            self.header_magic = header_magic;
            self.first_time_get_header = false;
        }

        match self.header_magic.as_str() {
            "WDBC" => EnumFileType::DbcFile,
            "WCH2" => EnumFileType::AdbFile,
            "WDB2" => EnumFileType::Db2File,
            "BDIW" => EnumFileType::WdbItemCacheFile,
            "BOMW" => EnumFileType::WdbCreatureCacheFile,
            "BOGW" => EnumFileType::WdbGameObjectCacheFile,
            "BDNW" => EnumFileType::WdbItemNameCacheFile,
            "XTIW" => EnumFileType::WdbItemTextCacheFile,
            "CPNW" => EnumFileType::WdbNpcCacheFile,
            "XTPW" => EnumFileType::WdbPageTextCacheFile,
            "TSQW" => EnumFileType::WdbQuestCacheFile,
            _ => { EnumFileType::UnkFile }
        }
    }
    fn check_structure(&mut self) -> bool {
        if self.get_file_type() == &EnumFileType::CsvFile || self.is_ascii {
            // todo: implement read csv files
        }
        else {
            match self.get_file_type_by_header() {
                EnumFileType::DbcFile => {
                    let header_size: u32 = 20;

                    let total_records = self.header_get_uint();
                    let total_fields = self.header_get_uint();
                    let record_size = self.header_get_uint();
                    let string_size = self.header_get_uint();

                    let data_bytes = self.file_size - header_size - string_size;
                    let string_bytes = self.file_size - header_size - data_bytes;

                    if data_bytes != (total_records * record_size) || string_size < 1 || string_bytes != string_size {
                        write_log_no_time!("FAILED: Structure is damaged.\n");
                        write_log!("\n");
                        return false
                    }

                    if total_records < 1 || total_fields < 1 || record_size < 1 {
                        write_log_no_time!("FAILED: No records found.\n");
                        write_log!("\n");
                        return false;
                    }

                }
                EnumFileType::AdbFile => {}
                EnumFileType::Db2File => {}

                EnumFileType::WdbCreatureCacheFile |
                EnumFileType::WdbGameObjectCacheFile |
                EnumFileType::WdbItemNameCacheFile |
                EnumFileType::WdbItemTextCacheFile |
                EnumFileType::WdbNpcCacheFile |
                EnumFileType::WdbPageTextCacheFile |
                EnumFileType::WdbQuestCacheFile => {}

                EnumFileType::WdbItemCacheFile => {}
                _ => {}
            }
        }

        true
    }
}