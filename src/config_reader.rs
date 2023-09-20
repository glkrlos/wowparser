use std::env;
use std::fs::File;
use std::io::Read;

use roxmltree::Document;
use serde::Deserialize;
use serde_xml_rs::from_str;

use crate::{write_log, write_log_and_print, write_log_no_time_and_print};
use crate::findfiles::instance as FindFiles;
use crate::log;
use crate::shared::{is_valid_format, OutputFormat, str_to};

static WOW_PARSER_XML: &str = "wowparser4.xml";

#[derive(Debug, Deserialize)]
struct Files {
    #[serde(rename = "name")]
    name: Option<String>,
    #[serde(rename = "extension")]
    extension: Option<String>,
    #[serde(rename = "recursive")]
    recursive: Option<String>,
    #[serde(rename = "directory")]
    directory: Option<String>,
    #[serde(rename = "format")]
    format: Option<String>,
    #[serde(rename = "ToCSV")]
    to_csv: Option<String>,
    #[serde(rename = "ToDBC")]
    to_dbc: Option<String>,
    #[serde(rename = "ToSQL")]
    to_sql: Option<String>,
}

#[derive(Debug, Deserialize)]
struct WoWParser {
    #[serde(rename = "file")]
    files: Option<Vec<Files>>,
}

pub struct ConfigReader {}

impl ConfigReader {
    pub fn new() -> Self {
        Self {}
    }

    pub fn load_configuration_file(&self) -> bool {
        write_log_and_print!("-----> Loading Configuration file... ");

        if !(WOW_PARSER_XML.chars().nth(12).unwrap() == 'm' &&
            WOW_PARSER_XML.chars().nth(4).unwrap() == 'a' &&
            WOW_PARSER_XML.chars().nth(7).unwrap() == 'e' &&
            WOW_PARSER_XML.chars().nth(8).unwrap() == 'r' &&
            WOW_PARSER_XML.chars().nth(13).unwrap() == 'l' &&
            WOW_PARSER_XML.chars().nth(2).unwrap() == 'w' &&
            WOW_PARSER_XML.chars().nth(9).unwrap() == '4' &&
            WOW_PARSER_XML.chars().nth(1).unwrap() == 'o' &&
            WOW_PARSER_XML.chars().nth(10).unwrap() == '.' &&
            WOW_PARSER_XML.chars().nth(0).unwrap() == 'w' &&
            WOW_PARSER_XML.chars().nth(5).unwrap() == 'r' &&
            WOW_PARSER_XML.chars().nth(3).unwrap() == 'p' &&
            WOW_PARSER_XML.chars().nth(6).unwrap() == 's' &&
            WOW_PARSER_XML.chars().nth(11).unwrap() == 'x')
        {
            write_log_no_time_and_print!("Configuration file has wrong name.\n");
            return false;
        }

        let current_dir = match env::current_exe() {
            Ok(dir) => dir,
            Err(_) => {
                write_log_no_time_and_print!("Unable to locate configuration file.\n");
                return false;
            }
        };

        let full_path = match current_dir.parent() {
            Some(dir) => dir.join(WOW_PARSER_XML),
            None => {
                write_log_no_time_and_print!("Unable to set configuration file.\n");
                return false;
            }
        };

        let mut file = match File::open(full_path) {
            Ok(file) => file,
            Err(_) => {
                write_log_no_time_and_print!("Unable to open configuration file.\n");
                return false;
            }
        };

        let mut xml_content = String::new();

        if !file.read_to_string(&mut xml_content).is_ok() {
            write_log_no_time_and_print!("Unable to read configuration file.\n");
            return false;
        }

        let document = match Document::parse(&xml_content) {
            Ok(data) => { data }
            Err(_) => {
                write_log_no_time_and_print!("Failed: Syntax errors.\n");
                return false;
            }
        };

        if !document.root_element().has_tag_name("WoWParser4") {
            write_log_no_time_and_print!("Failed: Invalid XML file.\n");
            return false;
        }

        let Ok(result): Result<WoWParser, serde_xml_rs::Error> = from_str(&xml_content)
            else {
                write_log_no_time_and_print!("Failed: Unserialize error.\n");
                return false;
            };

        let file_elements = match result.files {
            None => {
                write_log_no_time_and_print!("Failed: No files to parse.\n");
                return false;
            }
            Some(data) => { data }
        };

        write_log_no_time_and_print!("OK\n");

        write_log!("\n");
        write_log!("-----> Checking XML attributes of files to parse...\n");

        let mut file_id: u32 = 0;

        for file in file_elements {
            file_id += 1;

            let extension_attribute = file.extension.unwrap_or("".to_string());
            let extension_attribute_is_set = !extension_attribute.is_empty();

            let filename_attribute = file.name.unwrap_or("".to_string());
            let filename_is_set = !filename_attribute.is_empty();

            // Si no hay nombre continuamos
            if !filename_is_set && !extension_attribute_is_set {
                write_log!("\t WARNING: name attribute can't be empty in configuration file. Ignoring element number '{file_id}'\n");
                continue;
            }

            // si el valor de recursive no esta establecido o es un valor incorrecto entonces ponemos que recursive is not set
            let mut recursive_attribute = str_to::<bool>(file.recursive.unwrap_or(false.to_string()));

            let directory_attribute = file.directory.unwrap_or("".to_string());
            let directory_value = if directory_attribute.is_empty() { "." } else { &directory_attribute };

            // Si se establecio una extension de archivo y el atributo recursive no esta establecido entonces forzamos dicho modo
            if !recursive_attribute && extension_attribute_is_set {
                recursive_attribute = true;
            }

            let format_attribute = file.format.unwrap_or("".to_string());
            if !extension_attribute_is_set && !is_valid_format(&format_attribute) {
                write_log!("\t WARNING: For file name '{filename_attribute}' contains an invalid character in format attribute. Ignoring element '{file_id}'\n");
                continue;
            }

            let mut final_directory_value = String::from(directory_value);
            if final_directory_value == "." {
                final_directory_value += "/";
            }

            let output_formats = OutputFormat {
                is_set_to_csv: str_to::<bool>(file.to_csv.unwrap_or(true.to_string())),
                is_set_to_dbc: str_to::<bool>(file.to_dbc.unwrap_or(false.to_string())),
                is_set_to_sql: str_to::<bool>(file.to_sql.unwrap_or(false.to_string())),
            };

            FindFiles().file_to_find(
                final_directory_value.as_str(),
                filename_attribute.as_str(),
                format_attribute.as_str(),
                recursive_attribute,
                if extension_attribute_is_set { extension_attribute.as_str() } else { "" },
                output_formats,
            )
        }

        write_log!("-----> All OK after checking XML attributes of files to parse.\n");

        true
    }
}