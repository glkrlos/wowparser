use std::collections::HashMap;
use std::str::FromStr;
use crossterm::{event, terminal};
use serde::Serialize;

#[macro_export]
macro_rules! getch {
    () => {
            shared::getch();
    };
}

#[allow(dead_code)]
#[derive(Clone, Debug, Serialize)]
pub enum EnumFieldTypes {
    TypeNone    = 0,
    TypeString  = 1,
    TypeFloat   = 2,
    TypeByte    = 3,
    TypeInt     = 4,
    TypeUInt    = 5,
    TypeBool    = 6,
    TypeUByte   = 7,
}

#[allow(dead_code)]
#[derive(Clone, Debug, Serialize)]
pub enum EnumFileType {
    UnkFile                 = 0,
    DbcFile                 = 1,
    Db2File                 = 2,
    AdbFile                 = 3,
    WdbFile                 = 4,
    CsvFile                 = 5,
    TotalFileTypes          = 6,
    WdbItemCacheFile        = 7,
    WdbCreatureCacheFile    = 8,
    WdbGameObjectCacheFile  = 9,
    WdbItemNameCacheFile    = 10,
    WdbItemTextCacheFile    = 11,
    WdbNpcCacheFile         = 12,
    WdbPageTextCacheFile    = 13,
    WdbQuestCacheFile       = 14,
}

#[allow(non_snake_case)]
#[allow(dead_code)]
struct StructField {
    ID:             u32,
    Type:           EnumFieldTypes,
    StringValue:    u32,
    UIntValue:      u32,
    IntValue:       i32,
    BoolValue:      u32,
    ByteValue:      i8,
    UByteValue:     u8,
    FloatValue:     f32,
}

#[allow(dead_code)]
impl StructField {
    fn new() -> Self {
        Self {
            ID:             0,
            Type:           EnumFieldTypes::TypeNone,
            StringValue:    0,
            UIntValue:      0,
            IntValue:       0,
            BoolValue:      0,
            ByteValue:      0,
            UByteValue:     0,
            FloatValue:     0.0,
        }
    }
}

#[allow(dead_code)]
struct StructRecord {
    id:     u32,
    fields: Vec<StructField>,
}

#[allow(dead_code)]
pub struct StructFileData {
    records: Vec<StructRecord>,
}

#[allow(dead_code)]
#[derive(Clone, Copy, Debug, Serialize)]
pub struct OutputFormat {
    pub is_set_to_csv:  bool,
    pub is_set_to_dbc:  bool,
    pub is_set_to_sql:  bool,
}

#[allow(dead_code)]
impl OutputFormat {
    pub fn new() -> Self {
        Self {
            is_set_to_csv:  false,
            is_set_to_dbc:  false,
            is_set_to_sql:  false,
        }
    }
}

#[allow(dead_code)]
#[derive(Clone, Debug, Serialize)]
pub struct StructXMLFileInfo {
    pub file_name:                  String,
    pub file_type:                  EnumFileType,
    pub structure:                  String,
    pub is_recursively_searched:    bool,
    pub is_searched_by_extension:   bool,
    pub formatted_field_types:      Vec<EnumFieldTypes>,
    pub formatted_total_fields:     u32,
    pub formatted_record_size:      u32,
    pub output_formats:             OutputFormat,
}

#[allow(dead_code)]
impl StructXMLFileInfo {
    fn new() -> Self {
        Self {
            file_name:                  String::new(),
            file_type:                  EnumFileType::UnkFile,
            structure:                  String::new(),
            is_recursively_searched:    false,
            is_searched_by_extension:   false,
            formatted_field_types:      Vec::new(),
            formatted_total_fields:     0,
            formatted_record_size:      0,
            output_formats:             OutputFormat::new(),
        }
    }
}

#[allow(dead_code)]
pub struct SaveFileInfo {
    record_size:            u32,
    total_fields:           u32,
    total_records:          u32,
    field_types:            Vec<EnumFieldTypes>,
    saved_data:             HashMap<String, StructFileData>,
    string_size:            u32,
    string_texts:           String,
    unique_string_texts:    HashMap<String, Vec<u32>>,
}

#[allow(dead_code)]
impl SaveFileInfo {
    fn new() -> Self {
        Self {
            record_size:            0,
            total_fields:           0,
            total_records:          0,
            field_types:            Vec::new(),
            saved_data:             HashMap::new(),
            string_size:            1,
            string_texts:           String::from("\0"),
            unique_string_texts:    HashMap::new(),
        }
    }

    pub fn get_total_fields(&self) -> u32 {
        self.total_fields
    }

    pub fn get_total_records(&self) -> u32 {
        self.total_records
    }

    pub fn get_record_size(&self) -> u32 {
        self.record_size
    }

    pub fn get_string_size(&self) -> u32 {
        self.string_size
    }

    pub fn get_field_types(&self) -> &Vec<EnumFieldTypes> {
        &self.field_types
    }

    pub fn get_string_texts(&self) -> &String {
        &self.string_texts
    }

    pub fn get_unique_string_texts(&self) -> &HashMap<String, Vec<u32>> {
        &self.unique_string_texts
    }

    pub fn get_extracted_data(&self) -> &HashMap<String, StructFileData> {
        &self.saved_data
    }

    pub fn set_unique_string_texts(&mut self, text: &str) {
        if !text.is_empty() && self.get_unique_text_position(text) == 0 {
            let text_position = self.string_texts.len() as u32;
            self.string_texts.push_str(&(text.to_string() + "\0"));
            self.string_size += (text.len() + 1) as u32;

            let mut vector_for_text_position = Vec::new();
            vector_for_text_position.push(text_position);
            self.unique_string_texts.insert(text.to_string(), vector_for_text_position);
        }
    }

    pub fn get_unique_text_position(&self, text: &str) -> u32 {
        if !text.is_empty() {
            if let Some(find_text) = self.unique_string_texts.get(text) {
                if let Some(min_position) = find_text.iter().min() {
                    return *min_position;
                }
            }
        }

        0
    }

    pub fn set_unique_string_text_with_position(&mut self, text: String, text_position: u32) {
        if text.is_empty() {
            return;
        }

        if let Some(find_text) = self.unique_string_texts.get_mut(&text) {
            if !find_text.contains(&text_position) {
                find_text.push(text_position);
            }
        } else {
            let mut vector_for_text_position = Vec::new();
            vector_for_text_position.push(text_position);
            self.unique_string_texts.insert(text, vector_for_text_position);
        }
    }
}

pub fn getch() {
    println!("Press any key to continue...");
    let _ = terminal::enable_raw_mode().unwrap();

    //io::stdout().execute(terminal::Clear(ClearType::All)).unwrap();

    loop {
        if event::poll(std::time::Duration::from_millis(100)).unwrap() {
            if let event::Event::Key(event::KeyEvent {
                code: _,
                kind: _,
                state: _,
                modifiers: _,
            }) = event::read().unwrap()
            {
                /*if kind == KeyEventKind::Press && state == KeyEventState::NONE*/ {
                    break;
                }
            }
        }
    }

    let _ = terminal::disable_raw_mode().unwrap();
}

pub fn get_file_extension_by_file_type(eft: &str) -> EnumFileType {
    match eft {
        "dbc" => EnumFileType::DbcFile,
        "db2" => EnumFileType::Db2File,
        "adb" => EnumFileType::AdbFile,
        "wdb" => EnumFileType::WdbFile,
        "csv" => EnumFileType::CsvFile,
        _     => EnumFileType::UnkFile,
    }
}

pub fn str_to<T>(s: String) -> T
    where
        T: FromStr + Default,
{
    s.parse().unwrap_or_default()
}

pub fn is_valid_format(structure: &str) -> bool {
    for x in structure.chars() {
        match x {
            'X' |   // unk byte
            'b' |   // byte
            's' |   // string
            'f' |   // float
            'd' |   // int
            'n' |   // int
            'x' |   // unk int
            'i' |   // int
            'u' =>  // unsigned int
                continue,
            _ =>
                return false,
        }
    }
    true
}