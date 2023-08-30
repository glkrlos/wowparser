use lazy_static::lazy_static;
use std::sync::{Mutex, Arc};
use std::collections::HashMap;

#[macro_export]
macro_rules! getch {
    () => {
            println!("Press enter to continue...");
            let mut buffer = [0u8; 1];
            let stdin = io::stdin();

            stdin.lock().read(&mut buffer).unwrap();
    };
}

#[allow(dead_code)]
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
    fn new() -> StructField {
        StructField {
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
struct OutputFormat {
    to_csv:         bool,
    to_dbc:         bool,
    to_sql:         bool,
    is_set_to_csv:  bool,
    is_set_to_dbc:  bool,
    is_set_to_sql:  bool,
}

#[allow(dead_code)]
impl OutputFormat {
    fn new() -> Self {
        Self {
            to_csv:         false,
            to_dbc:         false,
            to_sql:         false,
            is_set_to_csv:  false,
            is_set_to_dbc:  false,
            is_set_to_sql:  false,
        }
    }
}

#[allow(dead_code)]
pub struct StructXMLFileInfo {
    file_name:                  String,
    file_type:                  EnumFileType,
    structure:                  String,
    is_recursively_searched:    bool,
    is_searched_by_extension:   bool,
    xml_file_id:                u32,
    formatted_field_types:      Vec<EnumFieldTypes>,
    formatted_total_fields:     u32,
    formatted_record_size:      u32,
    output_formats:             OutputFormat,
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
            xml_file_id:                0,
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

pub struct CShared;

#[allow(dead_code)]
impl CShared {
    pub fn show(&self) {
        println!("Shared");
    }

    pub fn to_str<T: ToString>(&self, i: T) -> String {
        i.to_string()
    }

    pub fn to_lower_case(&self, text: &str) -> String {
        text.to_lowercase()
    }

    pub fn compare_texts(txt1: &str, txt2: &str) -> bool {
        txt1 == txt2
    }

    pub fn get_file_extension_by_file_type(&self, eft: EnumFileType) -> &'static str {
        match eft {
            EnumFileType::DbcFile   => "dbc",
            EnumFileType::Db2File   => "db2",
            EnumFileType::AdbFile   => "adb",
            EnumFileType::WdbFile   => "wdb",
            EnumFileType::CsvFile   => "csv",
            _                       => "Unknown",
        }
    }
}

lazy_static! {
    static ref INSTANCE: Arc<Mutex<CShared>> = Arc::new(Mutex::new(CShared));
}

pub fn instance() -> std::sync::MutexGuard<'static, CShared> {
    INSTANCE.lock().unwrap()
}