use crate::shared::StructXMLFileInfo;

pub(crate) struct Parser {
    file_structure: StructXMLFileInfo
}

impl Parser {
    pub fn new(file_structure: StructXMLFileInfo) -> Self {
        Self { file_structure }
    }
    pub fn load(&self) -> bool {
        true
    }
    pub fn parse_file(&self){
        // just for ignoring unused struct
        let _ = self.file_structure;
    }
}