#ifndef _MODULE_CSV_WRITER_H_
#define _MODULE_CSV_WRITER_H_

#include "pch.h"
#include "shared.h"
#include "log.h"

class CSV_Writer
{
    public:
        CSV_Writer(string fileName,
                   vector<enumFieldTypes> fieldTypes,
                   map<string, structFileData> savedData,
                   string stringTexts) : _fileName(fileName), _fieldTypes(fieldTypes), _savedData(savedData), _stringTexts(stringTexts) {}
        ~CSV_Writer() {}
        bool CreateCSVFile();
    protected:
        string _fileName;
        vector<enumFieldTypes> _fieldTypes;
        map<string, structFileData> _savedData;
        string _stringTexts;
};

#endif