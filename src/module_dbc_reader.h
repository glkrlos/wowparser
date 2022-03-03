#ifndef _MODULE_DBC_READER_H_
#define _MODULE_DBC_READER_H_

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

class DBCReader : public GenericBinaryFileInfo
{
    public:
        DBCReader(const char *_FileName, vector<enumFieldTypes> _FieldTypes, unsigned int _FmtTotalFields, unsigned int _FmtRecordSize)
        {
            FileName = _FileName;
            FieldTypes = _FieldTypes;
            FormatedTotalFields = _FmtTotalFields;
            FormatedRecordSize = _FmtRecordSize;
            HeaderSize = sizeof(structDBCHeader);
            FileSize = 0;
        }
        bool Load();
    private:
        bool CheckStructure();
        bool PredictFieldTypes();
    protected:
        FILE *input;
};
#endif