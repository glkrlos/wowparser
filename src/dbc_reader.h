#ifndef _DBC_READER_H_
#define _DBC_READER_H_

#include "pch.h"
#include "shared.h"

struct structDBCHeader
{
    char header[4];
    unsigned int totalRecords;
    unsigned int totalFields;
    unsigned int recordSize;
    unsigned int stringSize;
};

class DBCReader : public BasicFileInfo
{
    public:
        DBCReader(const char *_FileName, vector<enumFieldTypes> _FieldTypes, unsigned int _FmtTotalFields, unsigned int _FmtRecordSize)
        {
            FileName = _FileName;
            FieldTypes = _FieldTypes;
            FormatedTotalFields = _FmtTotalFields;
            FormatedRecordSize = _FmtRecordSize;
            HeaderSize = sizeof(structDBCHeader);
        }
        bool Load();
    private:
        bool CheckStructure();
        bool PredictFieldTypes();
    protected:
        FILE *input;
};
#endif