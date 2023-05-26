#ifndef _MODULE_CSV_WRITER_H_
#define _MODULE_CSV_WRITER_H_

#include "pch.h"
#include "shared.h"

class CSV_Writer
{
    public:
        CSV_Writer(string fileName,
                   vector<enumFieldTypes> fieldTypes,
                   map<string, structFileData> savedData,
                   string stringTexts) : _fileName(std::move(fileName)), _fieldTypes(std::move(fieldTypes)), _savedData(std::move(savedData)), _stringTexts(std::move(stringTexts)) {}
        ~CSV_Writer() = default;
        bool CreateCSVFile();
    protected:
        string _fileName;
        vector<enumFieldTypes> _fieldTypes;
        map<string, structFileData> _savedData;
        string _stringTexts;
};

#endif