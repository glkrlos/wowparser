#ifndef MODULE_PARSER_H
#define MODULE_PARSER_H

#include "pch.h"
#include "shared.h"
#include "module_csv_reader.h"
#include "../lib/md5/md5.h"
#include "module_dbc_writer.h"
#include "module_csv_writer.h"
#include "module_sql_writer.h"

class PrintFileInfo
{
    public:
        PrintFileInfo(vector<enumFieldTypes> eFT, unsigned int totalFields, unsigned int totalRecords, bool predicted, string hash) : FieldTypes(std::move(eFT)), TotalFields(totalFields), TotalRecords(totalRecords), Predicted(predicted), Hash(hash)
        {
            _countFloatFields = 0;
            _countStringFields = 0;
            _countBoolFields = 0;
            _countByteFields = 0;
            _countUByteFields = 0;
            _countIntFields = 0;
            _countUIntFields = 0;
        }
        bool PrintResults()
        {
            for (auto & FieldType : FieldTypes)
            {
                switch (FieldType)
                {
                    case type_FLOAT:    _countFloatFields++; break;
                    case type_STRING:   _countStringFields++; break;
                    case type_BOOL:     _countBoolFields++; break;
                    case type_BYTE:     _countByteFields++; break;
                    case type_UBYTE:    _countUByteFields++; break;
                    case type_INT:      _countIntFields++; break;
                    case type_UINT:     _countUIntFields++; break;
                    default: break;
                }
            }

            if ((_countFloatFields + _countStringFields + _countBoolFields + _countByteFields + _countUByteFields + _countIntFields + _countUIntFields) != TotalFields)
            {
                Log->WriteLogNoTime("FAILED: One or more fields are not %s correctly. Conctact Developer to fix it.\n", Predicted ? "predicted" : "parsed");
                return false;
            }

            if (_countFloatFields)
                Log->WriteLog("Total float Fields%s: '%u'\n", Predicted ? " Predicted" : "", _countFloatFields);

            if (_countStringFields)
                Log->WriteLog("Total string Fields%s: '%u'\n", Predicted ? " Predicted" : "", _countStringFields);

            if (_countBoolFields)
                Log->WriteLog("Total bool Fields%s: '%u'\n", Predicted ? " Predicted" : "", _countBoolFields);

            if (_countByteFields)
                Log->WriteLog("Total byte Fields%s: '%u'\n", Predicted ? " Predicted" : "", _countByteFields);

            if (_countUByteFields)
                Log->WriteLog("Total unsigned byte Fields%s: '%u'\n", Predicted ? " Predicted" : "", _countUByteFields);

            if (_countIntFields)
                Log->WriteLog("Total int Fields%s: '%u'\n", Predicted ? " Predicted" : "", _countIntFields);

            if (_countUIntFields)
                Log->WriteLog("Total unsigned int Fields%s: '%u'\n", Predicted ? " Predicted" : "", _countUIntFields);

            if (TotalRecords)
                Log->WriteLog("Total records found: '%u'\n", TotalRecords);

            if (!Hash.empty())
                Log->WriteLog("MD5: %s\n", Hash.c_str());

            return true;
        }
    private:
    protected:
        unsigned int _countFloatFields;
        unsigned int _countStringFields;
        unsigned int _countBoolFields;
        unsigned int _countByteFields;
        unsigned int _countUByteFields;
        unsigned int _countIntFields;
        unsigned int _countUIntFields;
        vector<enumFieldTypes> FieldTypes;
        unsigned int TotalFields;
        unsigned int TotalRecords;
        bool Predicted;
        string Hash;
};

class DataAccessor : public SaveFileInfo
{
    public:
        DataAccessor()
        {
            _fieldsOffset = nullptr;
            _dataTable = nullptr;
            _stringTable = nullptr;
        }
        ~DataAccessor()
        = default;
        class RecordAccessor
        {
            public:
                [[nodiscard]] float GetFloat(size_t FieldID) const { return *reinterpret_cast<float *>(_data + _info.GetOffset(FieldID)); }
                [[nodiscard]] int GetInt(size_t FieldID) const { return *reinterpret_cast<int *>(_data + _info.GetOffset(FieldID)); }
                [[nodiscard]] unsigned int GetUInt(size_t FieldID) const { return *reinterpret_cast<unsigned int *>(_data + _info.GetOffset(FieldID)); }
                [[nodiscard]] unsigned int GetBool(size_t FieldID) const { return *reinterpret_cast<char *>(_data + _info.GetOffset(FieldID)); }
                [[nodiscard]] char GetByte(size_t FieldID) const { return *reinterpret_cast<char *>(_data + _info.GetOffset(FieldID)); }
                [[nodiscard]] unsigned char GetUByte(size_t FieldID) const { return *reinterpret_cast<unsigned char *>(_data + _info.GetOffset(FieldID)); }
                [[nodiscard]] const char *GetString(size_t FieldID) const { return reinterpret_cast<char *>(_info._stringTable + GetUInt(FieldID)); }
            private:
                RecordAccessor(DataAccessor &info, unsigned char *data) : _data(data), _info(info) { }
                unsigned char *_data = nullptr;
                DataAccessor &_info;
                friend class DataAccessor;
        };
        RecordAccessor GetRecord(size_t  RecordID) { return RecordAccessor(*this, _dataTable + RecordID * _recordSize); }
        [[nodiscard]] unsigned int GetOffset(size_t FieldID) const { return (_fieldsOffset != nullptr && FieldID < _totalFields) ? _fieldsOffset[FieldID] : 0; }
    protected:
        unsigned int *_fieldsOffset;
        unsigned char *_dataTable;
        unsigned char *_stringTable;
};

class Parser : public DataAccessor
{
    public:
        explicit Parser(structXMLFileInfo XMLFileInfo) : _XMLFileInfo(std::move(XMLFileInfo))
        {
            _inputFile = nullptr;
            _wholeFileData = nullptr;

            _fileSize = 0;

            _FirstTimeGetHeader = true;
            _isASCIIFile = false;
            _FirstTimeAksType = true;

            _headerMagic.clear();
            _headerOffset = 0;

            _stringsCount = 0;
            hash.clear();
        }
        ~Parser()
        {
            _inputFile = nullptr;

            if (_wholeFileData)
            {
                delete _wholeFileData;
                _wholeFileData = nullptr;
            }

            _fileSize = 0;

            _FirstTimeGetHeader = false;
            _isASCIIFile = false;
            _FirstTimeAksType = false;

            _headerMagic.clear();
            _headerOffset = 0;

            _stringsCount = 0;
            hash.clear();
        }
        bool Load();
        void ParseFile();
    private:
        bool ParseBinaryFile();
        bool ParseCSVFile();
        bool CheckStructure();
        bool PredictFieldTypes();
        [[nodiscard]] bool IsPreFormatted() const { return !_XMLFileInfo.FormatedFieldTypes.empty(); }
        [[nodiscard]] const char *GetFileName() const { return _XMLFileInfo.FileName.c_str(); }
        [[nodiscard]] enumFileType GetFileType() const { return _XMLFileInfo.Type; }
        bool FileIsASCII()
        {
            if (_FirstTimeAksType)
            {
                _FirstTimeAksType = false;

                bool binary = false;
                for (unsigned long x = 0; x < _fileSize; x++)
                {
                    if (static_cast<char>(_wholeFileData[x]) == '\n' || static_cast<char>(_wholeFileData[x]) == '\r')
                        continue;

                    if (!isprint(static_cast<char>(_wholeFileData[x])))
                    {
                        binary = true;
                        break;
                    }
                }

                if (binary)
                    _isASCIIFile = false;
                else
                    _isASCIIFile = true;
            }

            return _isASCIIFile;
        }
        enumFileType GetFileTypeByHeader()
        {
            if (_FirstTimeGetHeader)
            {
                _headerMagic = "";
                _headerMagic.append(Shared->ToStr(HeaderGetChar()));
                _headerMagic.append(Shared->ToStr(HeaderGetChar()));
                _headerMagic.append(Shared->ToStr(HeaderGetChar()));
                _headerMagic.append(Shared->ToStr(HeaderGetChar()));

                _FirstTimeGetHeader = false;
            }

            if (Shared->CompareTexts(_headerMagic, "WDBC"))
                return dbcFile;
            else if (Shared->CompareTexts(_headerMagic, "WCH2"))
                return adbFile;
            else if (Shared->CompareTexts(_headerMagic, "WDB2"))
                return db2File;
            else if (Shared->CompareTexts(_headerMagic, "BDIW"))
                return wdbitemcacheFile;
            else if (Shared->CompareTexts(_headerMagic, "BOMW"))
                return wdbcreaturecacheFile;
            else if (Shared->CompareTexts(_headerMagic, "BOGW"))
                return wdbgameobjectcacheFile;
            else if (Shared->CompareTexts(_headerMagic, "BDNW"))
                return wdbitemnamecacheFile;
            else if (Shared->CompareTexts(_headerMagic, "XTIW"))
                return wdbitemtextcacheFile;
            else if (Shared->CompareTexts(_headerMagic, "CPNW"))
                return wdbnpccacheFile;
            else if (Shared->CompareTexts(_headerMagic, "XTPW"))
                return wdbpagetextcacheFile;
            else if (Shared->CompareTexts(_headerMagic, "TSQW"))
                return wdbquestcacheFile;

            return unkFile;
        }
        bool NullPoniterToData() { return !_wholeFileData; }
        char HeaderGetChar() { return static_cast<char>(_wholeFileData[HeaderGetOffSet(1)]); }
        unsigned int HeaderGetUInt() { return *reinterpret_cast<unsigned int*>(_wholeFileData + HeaderGetOffSet(4)); }
        unsigned int HeaderGetOffSet(size_t size) { return (_headerOffset + size) <= _fileSize ? (_headerOffset += size) - size : 0; }

        void SetFieldTypesToNONE()
        {
            _fieldTypes.clear();
            for (unsigned int x = 0; x < _totalFields; x++)
                _fieldTypes.push_back(type_NONE);
        }
        void SetPredictedFieldsOffset()
        {
            _fieldsOffset = new unsigned int[_totalFields];
            _fieldsOffset[0] = 0;

            for (unsigned int i = 1; i < _totalFields; ++i)
                _fieldsOffset[i] = _fieldsOffset[i - 1] + 4;
        }
        void SetFormatedFieldsOffset()
        {
            _fieldsOffset = new unsigned int[_totalFields];
            _fieldsOffset[0] = 0;
            for (unsigned int i = 1; i < _totalFields; ++i)
            {
                _fieldsOffset[i] = _fieldsOffset[i - 1];
                if (_XMLFileInfo.FormatedFieldTypes[i - 1] == type_BYTE || _XMLFileInfo.FormatedFieldTypes[i - 1] == type_UBYTE)
                    _fieldsOffset[i] += 1;
                else
                    _fieldsOffset[i] += 4;
            }
        }
        void SetStringTextsFromStringTable(unsigned int StringBytes)
        {
            string Text;
            for (unsigned long currentChar = 1; currentChar < StringBytes; currentChar++)
            {
                char c = static_cast<char>(_stringTable[currentChar]);
                if (c == '\0')
                {
                    SetUniqueStringTexts(Text, currentChar - Text.size());
                    _stringTexts.append(Text + '\0');
                    _stringSize += Text.size() + 1;
                    Text.clear();
                    _stringsCount++;
                    continue;
                }

                Text.append(Shared->ToStr(c));
            }
        }
    private:
    protected:
        structXMLFileInfo _XMLFileInfo;

        FILE *_inputFile;
        unsigned char *_wholeFileData;

        unsigned long _fileSize;

        bool _FirstTimeGetHeader;
        bool _isASCIIFile;
        bool _FirstTimeAksType;

        string _headerMagic;
        unsigned int _headerOffset;

        unsigned int _stringsCount;

        string hash;
};
#endif
