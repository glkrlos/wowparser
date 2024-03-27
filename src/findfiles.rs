use lazy_static::lazy_static;
use std::sync::Mutex;
use std::collections::HashMap;
use crate::shared::{EnumFileType, OutputFormat, StructXMLFileInfo};

/*
TODO
 */
#[allow(non_camel_case_types)]
#[allow(non_snake_case)]
pub struct cFindFiles {
    fileName: HashMap<String, StructXMLFileInfo>
}

impl cFindFiles {
    pub fn new() -> Self {
        Self { fileName: HashMap::new() }
    }

    pub fn file_to_find(&self, _directory: &str, _filename: &str, _structure: &str, _recursive: bool, _file_ext: &str, _out_formats: OutputFormat, _xml_file_id: u32) {
        // println!(
        //     "\tDirectory: {}, Name: {}, Format: {}, Recursive: {}, Extension: {}, ToCSV: {}, ToDBC: {}, ToSQL: {}, File ID: {}",
        //     _directory,
        //     _filename,
        //     _structure,
        //     _recursive,
        //     _file_ext,
        //     _out_formats.is_set_to_csv,
        //     _out_formats.is_set_to_dbc,
        //     _out_formats.is_set_to_sql,
        //     _xml_file_id
        // );
    }

    pub fn list_empty(&self) -> bool {
        false
    }
    
    pub fn print_all_file_names_by_file_type(&self) {
    }

    pub fn xml_file_info(&mut self) -> HashMap<String, StructXMLFileInfo> {
        /*
        TODO: Just for testing purposes
         */
        self.fileName.insert(String::from("file1.wdb"), StructXMLFileInfo{
            file_name: "".to_string(),
            file_type: EnumFileType::UnkFile,
            structure: "".to_string(),
            is_recursively_searched: false,
            is_searched_by_extension: false,
            xml_file_id: 0,
            formatted_field_types: vec![],
            formatted_total_fields: 0,
            formatted_record_size: 0,
            output_formats: OutputFormat::new(),
        });
        self.fileName.insert(String::from("file2.wdb"), StructXMLFileInfo{
            file_name: "".to_string(),
            file_type: EnumFileType::UnkFile,
            structure: "".to_string(),
            is_recursively_searched: false,
            is_searched_by_extension: false,
            xml_file_id: 0,
            formatted_field_types: vec![],
            formatted_total_fields: 0,
            formatted_record_size: 0,
            output_formats: OutputFormat::new(),
        });

        self.fileName.clone()
    }
}

lazy_static! {
    static ref INSTANCE: Mutex<cFindFiles> = Mutex::new(cFindFiles::new());
}

pub fn instance() -> std::sync::MutexGuard<'static, cFindFiles> {
    INSTANCE.lock().unwrap()
}