#ifndef _MODULE_DBC_WRITER_H_
#define _MODULE_DBC_WRITER_H_

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
            _stringTexts = stringTexts;
            _fileName = fileName;
            _savedData = savedData;
        }
        ~DBC_Writer() { }
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