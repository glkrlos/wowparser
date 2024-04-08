#ifndef MODULE_DBC_WRITER_H
#define MODULE_DBC_WRITER_H

#include "pch.h"
#include "shared.h"

class DBC_Writer
{
    public:
        DBC_Writer(unsigned int totalRecords,
                   unsigned int totalFields,
                   unsigned int recordSize,
                   unsigned int stringSize,
                   string stringTexts,
                   string fileName,
                   map<string, structFileData> savedData)
        {
            _totalRecords = totalRecords;
            _totalFields = totalFields;
            _recordSize = recordSize;
            _stringSize = stringSize;
            _stringTexts = std::move(stringTexts);
            _fileName = std::move(fileName);
            _savedData = std::move(savedData);
        }
        ~DBC_Writer() = default;
        bool CreateDBCFile();
    protected:
        unsigned int _totalRecords;
        unsigned int _totalFields;
        unsigned int _recordSize;
        unsigned int _stringSize;
        string _stringTexts;
        string _fileName;
        map<string, structFileData> _savedData;
};

#endif