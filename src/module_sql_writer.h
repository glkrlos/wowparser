#ifndef MODULE_SQL_WRITER_H
#define MODULE_SQL_WRITER_H

#include "pch.h"
#include "shared.h"

class SQL_Writer
{
    public:
        SQL_Writer(string fileName,
                   vector<enumFieldTypes> fieldTypes,
                   map<string, structFileData> savedData,
                   string stringTexts) : _fileName(fileName), _fieldTypes(fieldTypes), _savedData(savedData), _stringTexts(stringTexts) {}
        ~SQL_Writer() {}
        bool CreateSQLFile();
    protected:
        string _fileName;
        vector<enumFieldTypes> _fieldTypes;
        map<string, structFileData> _savedData;
        string _stringTexts;
};

#endif