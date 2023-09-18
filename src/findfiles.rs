use lazy_static::lazy_static;
use std::sync::Mutex;
use std::collections::HashMap;
use std::fs;
use std::path::Path;
use crate::shared::{EnumFieldTypes, EnumFileType, OutputFormat, StructXMLFileInfo};
use crate::shared::get_file_extension_by_file_type as GetFileExtensionByFileType;

pub struct FindFiles {
    file_name: HashMap<String, StructXMLFileInfo>
}

impl FindFiles {
    pub fn new() -> Self {
        Self { file_name: HashMap::new() }
    }

    pub fn file_to_find(&mut self, directory: &str, filename: &str, structure: &str, recursive: bool, file_ext: &str, out_formats: OutputFormat) {

        // If unable to read directory, returns
        let current_directory = match fs::read_dir(Path::new(directory)) {
            Ok(data) => { data }
            Err(_) => { return }
        };

        for entry in current_directory {

            // If unable to check validity of path entry, continues
            let current_entry = match entry {
                Ok(entry) => { entry.path() }
                Err(_) => { continue }
            };

            if current_entry.is_dir() {

                if recursive {
                    self.file_to_find(&current_entry.to_string_lossy(), &filename, &structure, recursive, &file_ext, out_formats);
                }

                continue;
            }

            // If no file extension, continues
            let file_extension = match current_entry.extension() {
                None => { continue }
                Some(data) => { data.to_str() }
            };

            // If unable to get string from extension, continues
            let current_extension = match file_extension {
                None => { continue }
                Some(data) => { data.to_string() }
            };

            let mut info = StructXMLFileInfo {
                file_name: current_entry.display().to_string(),
                file_type: EnumFileType::UnkFile,
                structure: "".to_string(),
                is_recursively_searched: recursive,
                is_searched_by_extension: false,
                formatted_field_types: vec![],
                formatted_total_fields: 0,
                formatted_record_size: 0,
                output_formats: out_formats,
            };

            // If set file extension is on, equal compares, else continue
            if !file_ext.is_empty() {
                // If entry extension is different from typed extension, continues
                if current_extension.to_lowercase() != file_ext.to_lowercase() {
                    continue
                }

                info.file_type = GetFileExtensionByFileType(file_ext.to_lowercase().as_str());
                info.is_searched_by_extension = true;
                self.add_file_to_list_if_not_exist(&current_entry.display().to_string(), info.clone());
            }
            else {
                // If unable to get file name part, continues
                let current_entry_file_name_option = match current_entry.file_name() {
                    None => { continue }
                    Some(data) => { data.to_str() }
                };

                let current_entry_file_name = match current_entry_file_name_option {
                    None => { continue }
                    Some(data) => { data.to_string() }
                };

                // If current entry file name mismatch with the set filename, continues
                if current_entry_file_name.to_lowercase() != filename.to_lowercase() {
                    continue
                }

                info.file_type = GetFileExtensionByFileType(current_extension.to_lowercase().as_str());
                info.structure = structure.to_string();
                info.is_searched_by_extension = false;
                info.formatted_field_types = self.get_formated_field_types(structure);
                info.formatted_total_fields = structure.len() as u32;
                info.formatted_record_size = self.get_formated_record_size(structure);

                self.add_file_to_list_if_not_exist(&current_entry.display().to_string(), info.clone());
            }
        }
    }

    fn get_formated_record_size(&self, frs: &str) -> u32 {
        let mut record_size = 0;

        for x in frs.chars() {
            match x {
                'X' |   // unk byte
                'b'     // byte
                        => record_size += 1,
                        // any other
                _       => record_size += 4,
            }
        }

        record_size
    }
    fn get_formated_field_types(&self, fft: &str) -> Vec<EnumFieldTypes>{
        let mut field_types = Vec::new();

        for x in fft.chars() {
            match x {
                'X' |   // unk byte
                'b'     // byte
                        => field_types.push(EnumFieldTypes::TypeByte),
                's'     // string
                        => field_types.push(EnumFieldTypes::TypeString),
                'f'     // float
                        => field_types.push(EnumFieldTypes::TypeFloat),
                'd' |   // int
                'n' |   // int
                'x' |   // unk int
                'i'     // int
                        => field_types.push(EnumFieldTypes::TypeInt),
                'u'     // unsigned int
                        => field_types.push(EnumFieldTypes::TypeUInt),
                        // none
                _       => field_types.push(EnumFieldTypes::TypeNone),
            }
        }

        field_types
    }

    pub fn list_empty(&self) -> bool {
        self.file_name.is_empty()
    }

    fn add_file_to_list_if_not_exist(&mut self, file_name: &str, file_info: StructXMLFileInfo)
    {
        if self.file_name.contains_key(file_name) {
            self.file_name.insert(file_name.to_string(), file_info);

            return;
        }

        self.file_name.insert(file_name.to_string(), file_info);
    }

    pub fn print_all_file_names_by_file_type(&self) {
        let json_string = serde_json::to_string_pretty(&self.file_name).unwrap();
        println!("{}", json_string);
    }

    pub fn xml_file_info(&self) -> HashMap<String, StructXMLFileInfo> {
        self.file_name.clone()
    }
}

lazy_static! {
    static ref INSTANCE: Mutex<FindFiles> = Mutex::new(FindFiles::new());
}

pub fn instance() -> std::sync::MutexGuard<'static, FindFiles> {
    INSTANCE.lock().unwrap()
}