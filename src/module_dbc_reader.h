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

class BasicFileInfo
{
    public:
        const char *FileName;
        unsigned int HeaderSize;
        unsigned int TotalRecords;
        unsigned int TotalFields;
        unsigned int RecordSize;
        unsigned int StringSize;
        unsigned char *Data;
        unsigned char *StringTable;
        long FileSize;
        unsigned int FormatedTotalFields;
        unsigned int FormatedRecordSize;
        vector<enumFieldTypes> FieldTypes;
        bool isFormated() { return !FieldTypes.empty(); }

        void SetFieldTypesToNONE()
        {
            FieldTypes.clear();
            for (unsigned int x = 0; x < TotalFields; x++)
                FieldTypes.push_back(type_NONE);
        }
        void SetFieldsOffset()
        {
            _fieldsOffset = new unsigned int[TotalFields];
            _fieldsOffset[0] = 0;
            for (unsigned int i = 1; i < TotalFields; ++i)
            {
                _fieldsOffset[i] = _fieldsOffset[i - 1];
                if (FieldTypes[i - 1] == type_BYTE)
                    _fieldsOffset[i] += 1;
                else
                    _fieldsOffset[i] += 4;
            }
        }
        class Record
        {
            public:
                float GetFloat(size_t FieldID) const { return *reinterpret_cast<float*>(_data + _info.GetOffset(FieldID)); }
                int GetInt(size_t FieldID) const { return *reinterpret_cast<int*>(_data + _info.GetOffset(FieldID)); }
                unsigned int GetUInt(size_t FieldID) const { return *reinterpret_cast<unsigned int*>(_data + _info.GetOffset(FieldID)); }
                unsigned int GetBool(size_t FieldID) const { return GetUInt(FieldID); }
                char GetByte(size_t FieldID) const { return *reinterpret_cast<char *>(_data + _info.GetOffset(FieldID)); }
                const char *GetString(size_t FieldID) const { return reinterpret_cast<char*>(_info.StringTable + GetUInt(FieldID)); }
            private:
                Record(BasicFileInfo &info, unsigned char *data) : _data(data), _info(info) { }
                unsigned char *_data;
                BasicFileInfo &_info;
                friend class BasicFileInfo;
            };
        unsigned int GetOffset(size_t FieldID) const { return (_fieldsOffset != NULL && FieldID < TotalFields) ? _fieldsOffset[FieldID] : 0; }
        Record GetRecord(size_t  RecordID) { return Record(*this, Data + RecordID * RecordSize); }
    protected:
        unsigned int *_fieldsOffset;
        unsigned int _recordOffset;
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