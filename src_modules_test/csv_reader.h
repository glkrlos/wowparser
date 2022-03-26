#ifndef _CSV_READER_H_
#define _CSV_READER_H_

#include "pch.h"
#include "shared.h"

class CSV_Reader : public SaveFileInfo
{
    public:
        CSV_Reader(const char *);
        ~CSV_Reader();
        bool LoadCSVFile();
        FileData *ExportFileData() { return new FileData(_structFileInfo, fileData); };

    private:
        bool ExtractFields(string, map<unsigned int, string> &);
        map<unsigned int, string> GetFields(string);
        bool SetFieldTypes(string);
        enumFieldTypes GetFieldType(unsigned int);
        bool CheckFieldsOfEachRecordAndSaveAllData(map<unsigned int, string>);
        bool CheckFieldValue(unsigned int, enumFieldTypes, string, unsigned int);
        const char* GetFieldTypeName(enumFieldTypes);
    protected:
        const char * fileName;
        unsigned int recordSize;
        unsigned int totalRecords;
        unsigned int totalFields;

        map<unsigned int, map<unsigned int, structRecord>> fileData;
        vector<enumFieldTypes> fieldTypes;
};

#endif _CSV_READER_H_