#ifndef _MODULE_CSV_READER_H_
#define _MODULE_CSV_READER_H_

#include "pch.h"
#include "shared.h"

class CSV_Reader : public SaveFileInfo
{
    public:
        CSV_Reader(const char *, map<unsigned int, string>);
        ~CSV_Reader();
        bool CheckCSV();
        bool CheckFieldsOfEachRecordAndSaveAllData();
    private:
        static bool ExtractFields(string, map<unsigned int, string> &);
        bool SetFieldTypes(const string&);
        enumFieldTypes GetFieldType(unsigned int);
        static const char* GetFieldTypeName(enumFieldTypes);

        static bool CheckFieldValue(unsigned int, enumFieldTypes, string, unsigned int);
    protected:
        map<unsigned int, string> _fileData;
        const char *_fileName;
};

#endif