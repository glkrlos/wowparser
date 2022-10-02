#ifndef _MODULE_PARSER_H_
#define _MODULE_PARSER_H_

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

class module_parser
{
    public:
        module_parser(const char *FileName, vector<enumFieldTypes> FieldTypes, unsigned int FmtTotalFields, unsigned int FmtRecordSize)
        {
            _fileName = FileName;
            _fieldTypes = FieldTypes;
            _formatedTotalFields = FmtTotalFields;
            _formatedRecordSize = FmtRecordSize;
            _headerSize = sizeof(structDBCHeader);
        }
        ~module_parser()
        {
            if (_inputFile)
                fclose(_inputFile);
        }
        bool Load();
    private:
        bool CheckStructure();
        bool PredictFieldTypes();

        bool isFormated() { return !_fieldTypes.empty(); }
        void SetFieldTypesToNONE()
        {
            _fieldTypes.clear();
            for (unsigned int x = 0; x < _totalFields; x++)
                _fieldTypes.push_back(type_NONE);
        }
        void SetFieldsOffset()
        {
            _fieldsOffset = new unsigned int[_totalFields];
            _fieldsOffset[0] = 0;
            for (unsigned int i = 1; i < _totalFields; ++i)
            {
                _fieldsOffset[i] = _fieldsOffset[i - 1];
                if (_fieldTypes[i - 1] == type_BYTE)
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
                const char *GetString(size_t FieldID) const { return reinterpret_cast<char*>(_info._stringTable + GetUInt(FieldID)); }
            private:
                RecordAccessor(module_parser &info, unsigned char *data) : _data(data), _info(info) { }
                unsigned char *_data = NULL;
                module_parser &_info;
                friend class module_parser;
        };
        RecordAccessor GetRecord(size_t  RecordID) { return RecordAccessor(*this, _dataTable + RecordID * _recordSize); }
        unsigned int GetOffset(size_t FieldID) const { return (_fieldsOffset != NULL && FieldID < _totalFields) ? _fieldsOffset[FieldID] : 0; }
    protected:
        FILE *_inputFile;
        const char *_fileName = NULL;
        long _fileSize = 0;
        unsigned int _headerSize = 0;
        unsigned int _totalRecords = 0;
        unsigned int _totalFields = 0;
        unsigned int _recordSize = 0;
        unsigned int _stringSize = 0;
        long _dataBytes = 0;
        long _stringBytes = 0;
        long _unkBytes = 0;
        vector<enumFieldTypes> _fieldTypes;
        unsigned int _formatedTotalFields = 0;
        unsigned int _formatedRecordSize = 0;
        unsigned int *_fieldsOffset = NULL;
        unsigned char *_dataTable = NULL;
        unsigned char *_stringTable = NULL;
};
#endif