use lazy_static::lazy_static;
use std::sync::Mutex;
use std::collections::HashMap;
use std::fs;
use std::path::Path;
use crate::{log, write_log, write_log_no_time};
use crate::shared;
use crate::shared::{EnumFieldTypes, EnumFileType, OutputFormat, StructXMLFileInfo};

pub struct FindFiles {
    file_names: HashMap<String, StructXMLFileInfo>
}

impl FindFiles {
    pub fn new() -> Self {
        Self { file_names: HashMap::new() }
    }

    pub fn file_to_find(&mut self, directory: &str, filename: &str, structure: &str, recursive: bool, file_ext: &str, out_formats: OutputFormat, file_id: u32) {

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
                    self.file_to_find(&current_entry.to_string_lossy(), &filename, &structure, recursive, &file_ext, out_formats, file_id);
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
                file_extension: current_extension.to_ascii_lowercase(),
                file_id,
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

                info.file_type = self.get_file_type_by_extension(file_ext.to_lowercase().as_str());
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

                info.file_type = self.get_file_type_by_extension(current_extension.to_lowercase().as_str());
                info.structure = structure.to_string();
                info.is_searched_by_extension = false;
                info.formatted_field_types = self.get_formated_field_types(structure);
                info.formatted_total_fields = structure.len() as u32;
                info.formatted_record_size = self.get_formated_record_size(structure);

                self.add_file_to_list_if_not_exist(&current_entry.display().to_string(), info.clone());
            }
        }
    }

    pub fn get_file_type_by_extension(&self, eft: &str) -> EnumFileType {
        match eft {
            "dbc" => EnumFileType::DbcFile,
            "db2" => EnumFileType::Db2File,
            "adb" => EnumFileType::AdbFile,
            "wdb" => EnumFileType::WdbFile,
            "csv" => EnumFileType::CsvFile,
            _     => EnumFileType::UnkFile,
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
        self.file_names.is_empty()
    }

    fn add_file_to_list_if_not_exist(&mut self, file_name: &str, file_info: StructXMLFileInfo) {
        if let Some(found) = self.file_names.get_mut(file_name) {
            *found = file_info;

            return;
        }

        self.file_names.insert(file_name.to_string(), file_info);
    }

    pub fn print_all_file_names_by_file_type(&self) {
        if self.list_empty() {
            return
        }

        let mut max_file_id_in_xml = 0;

        for file in &self.file_names {
            if file.1.file_id > max_file_id_in_xml {
                max_file_id_in_xml = file.1.file_id;
            }
        }

        for current_file_id in 0..=max_file_id_in_xml {
            for x in 0..(EnumFileType::TotalFileTypes as u32) {
                let mut count_current_files = 0;

                for file in &self.file_names {
                    if (file.1.file_type.clone() as u32) != x || file.1.file_id != current_file_id {
                        continue;
                    }

                    count_current_files += 1;
                }

                let mut first = true;

                for file in &self.file_names {
                    if (file.1.file_type.clone() as u32) != x || file.1.file_id != current_file_id {
                         continue
                    }

                    if first {
                        write_log!("->{} '{}' {} file{} added",
                               if file.1.file_type == EnumFileType::UnkFile { "(WARNING)" } else { "" },
                               count_current_files,
                               shared::get_file_extension_by_file_type(&file.1.file_type),
                               if count_current_files > 1 { "s" } else { "" });

                        if file.1.is_searched_by_extension {
                            write_log_no_time!(" with extension *.{}{}",
                                if file.1.file_type == EnumFileType::UnkFile { &file.1.file_extension } else { shared::get_file_extension_by_file_type(&file.1.file_type) },
                                if file.1.is_recursively_searched { " in recursive mode" } else { "" } );
                        }

                        if file.1.file_id > 0 {
                            write_log_no_time!(" by <file> element '{}'", file.1.file_id);
                        }

                        if file.1.is_searched_by_extension {
                            write_log_no_time!(", and they will pass to predicted mode");
                        }

                        if file.1.output_formats.is_set_to_csv || file.1.output_formats.is_set_to_dbc || file.1.output_formats.is_set_to_sql {
                            write_log_no_time!(" with output to");

                            let mut count_outputs: u32 = 0;

                            if file.1.output_formats.is_set_to_csv {
                                write_log_no_time!(" CSV");
                                count_outputs += 1;
                            }

                            if file.1.output_formats.is_set_to_dbc {
                                if file.1.output_formats.is_set_to_csv && file.1.output_formats.is_set_to_sql {
                                    write_log_no_time!(",");
                                }
                                else if file.1.output_formats.is_set_to_csv && !file.1.output_formats.is_set_to_sql {
                                    write_log_no_time!(" and");
                                }

                                write_log_no_time!(" DBC");
                                count_outputs += 1;
                            }

                            if file.1.output_formats.is_set_to_sql {
                                if file.1.output_formats.is_set_to_csv || file.1.output_formats.is_set_to_dbc {
                                    write_log_no_time!(" and");
                                }

                                write_log_no_time!(" SQL");
                                count_outputs += 1;
                            }

                            write_log_no_time!(" file format{}",
                                if count_outputs > 1 { "s" } else { "" });
                        }
                        else {
                            write_log_no_time!(" with file information only");
                        }

                        write_log_no_time!(".\n");
                        first = false;
                    }

                    write_log!("File: '{}'", file.1.file_name);

                    if !file.1.structure.is_empty() {
                        write_log_no_time!(", Structure: '{}'", file.1.structure);
                    }

                    write_log_no_time!("\n");
                }

            }
        }
    }

    pub fn xml_file_info(&self) -> HashMap<String, StructXMLFileInfo> {
        self.file_names.clone()
    }
}

lazy_static! {
    static ref INSTANCE: Mutex<FindFiles> = Mutex::new(FindFiles::new());
}

pub fn instance() -> std::sync::MutexGuard<'static, FindFiles> {
    INSTANCE.lock().unwrap()
}