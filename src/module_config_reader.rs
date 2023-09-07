use std::env;
use std::fs::File;
use std::io::Read;
use roxmltree::Document;

use crate::{write_log, write_log_and_print, write_log_no_time_and_print};
use crate::log;
use crate::shared::{is_valid_format, OutputFormat, str_to_type};
use crate::findfiles::instance as FindFiles;

static WOW_PARSER_XML: &str = "wowparser4.xml";

pub struct ConfigReader {
}

impl ConfigReader {
    pub fn new() -> Self {
        Self { }
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
            return false
        }

        let current_dir = match env::current_exe() {
            Ok(dir) => dir,
            Err(_e) => {
                write_log_no_time_and_print!("Unable to locate configuration file.\n");
                return false
            }
        };

        let full_path = match current_dir.parent() {
            Some(dir) => dir.join(WOW_PARSER_XML),
            None => {
                write_log_no_time_and_print!("Unable to set configuration file.\n");
                return false
            }
        };

        let mut file = match File::open(full_path) {
            Ok(file) => file,
            Err(_e) => {
                write_log_no_time_and_print!("Unable to open configuration file.\n");
                return false
            }
        };

        let mut xml_content = String::new();

        if !file.read_to_string(&mut xml_content).is_ok() {
            write_log_no_time_and_print!("Unable to read configuration file.\n");
            return false
        }

        let document = match Document::parse(&xml_content) {
            Ok(data) => { data }
            Err(_) => {
                write_log_no_time_and_print!("Failed: Syntax errors.\n");
                return false
            } 
        };

        if !document.root_element().has_tag_name("WoWParser4") {
            write_log_no_time_and_print!("Failed: Invalid XML file.\n");
            return false
        }

        let root = document.root_element();
        let file_count = root.children().filter(|n| n.has_tag_name("file")).count();

        if file_count == 0 {
            write_log_no_time_and_print!("Failed: No files to parse.\n");
            return false
        }

        write_log_no_time_and_print!("OK\n");

        write_log!("\n");
        write_log!("-----> Checking XML attributes of files to parse...\n");

        let mut file_id: u32 = 0;

        for child in root.children() {
            if !child.has_tag_name("file") {
                continue
            }

            file_id += 1;

            let extension_attribute = child.attribute("extension").unwrap_or("");
            let file_extension_is_set = !extension_attribute.is_empty();

            let name_attribute = child.attribute("name").unwrap_or("");
            let name = !name_attribute.is_empty();

            // Si no hay nombre continuamos
            if !name && !file_extension_is_set {
                write_log!("\t WARNING: name attribute can't be empty in configuration file. Ignoring element number '{file_id}'\n");
                continue
            }

            let mut is_recursive = false;
            // si el valor de recursive no esta establecido o es un valor incorrecto entonces ponemos que recursive is not set
            let recursive_attribute = str_to_type::<bool>(child.attribute("recursive").unwrap_or("false"));

            let directory_attribute = child.attribute("directory").unwrap_or("");
            let directory = if directory_attribute.is_empty() { "." } else { directory_attribute };

            // Si se establecio una extension de archivo y el atributo recursive no esta establecido entonces forzamos dicho modo
            if !recursive_attribute && file_extension_is_set {
                is_recursive = true;
            }

            let format_attribute = child.attribute("format").unwrap_or("");

            if !file_extension_is_set && !is_valid_format(format_attribute) {
                write_log!("\t WARNING: For file name '{name_attribute}' contains an invalid character in format attribute. Ignoring element '{file_id}'\n");
                continue
            }

            let mut temp_directory = String::from(directory);
            if temp_directory == "." {
                temp_directory += "/";
            }

            let to_csv = str_to_type::<bool>(child.attribute("ToCSV").unwrap_or("true"));
            let to_dbc = str_to_type::<bool>(child.attribute("ToDBC").unwrap_or("false"));
            let to_sql = str_to_type::<bool>(child.attribute("ToSQL").unwrap_or("false"));

            let out_formats = OutputFormat {
                is_set_to_csv: to_csv,
                is_set_to_dbc: to_dbc,
                is_set_to_sql: to_sql,
            };

            FindFiles().file_to_find(
                &*temp_directory,
                name_attribute,
                format_attribute,
                is_recursive,
                if file_extension_is_set { extension_attribute } else { "" },
                out_formats,
                file_id
            )
        }

        write_log!("-----> All OK after checking XML attributes of files to parse.\n");

        true
    }
}