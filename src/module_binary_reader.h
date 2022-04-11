#ifndef _MODULE_BINARY_READER_H_
#define _MODULE_BINARY_READER_H_

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

class BinaryDataAccessor
{
    public:
        const char *FileName = NULL;
        long FileSize = 0;
        unsigned int HeaderSize = 0;
        unsigned int TotalRecords = 0;
        unsigned int TotalFields = 0;
        unsigned int RecordSize = 0;
        unsigned int StringSize = 0;
        long DataBytes = 0;
        long StringBytes = 0;
        long UnkBytes = 0;
        unsigned int FormatedTotalFields = 0;
        unsigned int FormatedRecordSize = 0;

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

        class RecordAccessor
        {
            public:
                float GetFloat(size_t FieldID) const { return *reinterpret_cast<float*>(_data + _info.GetOffset(FieldID)); }
                int GetInt(size_t FieldID) const { return *reinterpret_cast<int*>(_data + _info.GetOffset(FieldID)); }
                unsigned int GetUInt(size_t FieldID) const { return *reinterpret_cast<unsigned int*>(_data + _info.GetOffset(FieldID)); }
                unsigned int GetBool(size_t FieldID) const { return GetUInt(FieldID); }
                char GetByte(size_t FieldID) const { return *reinterpret_cast<char *>(_data + _info.GetOffset(FieldID)); }
                const char *GetString(size_t FieldID) const { return reinterpret_cast<char*>(_info.StringTable + GetUInt(FieldID)); }
            private:
                RecordAccessor(BinaryDataAccessor &info, unsigned char *data) : _data(data), _info(info) { }
                unsigned char *_data = NULL;
                BinaryDataAccessor &_info;
                friend class BinaryDataAccessor;
        };
        unsigned int GetOffset(size_t FieldID) const { return (_fieldsOffset != NULL && FieldID < TotalFields) ? _fieldsOffset[FieldID] : 0; }
        RecordAccessor GetRecord(size_t  RecordID) { return RecordAccessor(*this, DataTable + RecordID * RecordSize); }
    protected:
        unsigned int *_fieldsOffset = NULL;
        unsigned int _recordOffset = 0;
        unsigned char *DataTable = NULL;
        unsigned char *StringTable = NULL;
};

class BinaryReader : public BinaryDataAccessor
{
    public:
        BinaryReader(const char *_FileName, vector<enumFieldTypes> _FieldTypes, unsigned int _FmtTotalFields, unsigned int _FmtRecordSize)
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