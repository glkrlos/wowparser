#ifndef _MODULE_PARSER_H_
#define _MODULE_PARSER_H_

#include "pch.h"
#include "shared.h"
#include "module_csv_reader.h"
#include "ProgressBar.h"

class PrintFileInfo
{
    public:
        PrintFileInfo(vector<enumFieldTypes> eFT, unsigned int totalFields, bool predicted) : FieldTypes(eFT), TotalFields(totalFields), Predicted(predicted) { }
        bool PrintFileInfo::PrintResults()
        {
            for (auto it = FieldTypes.begin(); it != FieldTypes.end(); ++it)
            {
                switch (*it)
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

            return true;
        }
    private:
    protected:
        unsigned int _countFloatFields = 0;
        unsigned int _countStringFields = 0;
        unsigned int _countBoolFields = 0;
        unsigned int _countByteFields = 0;
        unsigned int _countUByteFields = 0;
        unsigned int _countIntFields = 0;
        unsigned int _countUIntFields = 0;
        vector<enumFieldTypes> FieldTypes;
        unsigned int TotalFields = 0;
        bool Predicted = false;
};

class module_parser : public SaveFileInfo
{
    public:
        module_parser(map<string, structFileInfo> files) : _ListOfAllFilesToParse(files) {}
        module_parser(structFileInfo sFile)
        {
            _fileName = sFile.FileName;
            _fileType = sFile.Type;
            _formatedFieldTypes = sFile.FormatedFieldTypes;
            _formatedTotalFields = sFile.FormatedTotalFields;
            _formatedRecordSize = sFile.FormatedRecordSize;
        }
        ~module_parser()
        {
            _header.clear();

            if (_wholeFileData)
            {
                delete _wholeFileData;
                _wholeFileData = NULL;
            }

            if (_inputFile) fclose(_inputFile);
        }
        bool Load();
        void ParseFile();
        void CheckHeadersAndDataConsistencyOfAllFilesAdded();
    private:
        bool ParseBinaryFile();
        bool ParseCSVFile();
        bool CheckStructure();
        bool PredictFieldTypes();
        bool IsPreFormatted() { return !_formatedFieldTypes.empty(); }
        const char *GetFileName() { return _fileName.c_str(); }
        enumFileType GetFileType() { return _fileType; }
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
                _header = "";
                _header.append(Shared->ToStr(HeaderGetChar()));
                _header.append(Shared->ToStr(HeaderGetChar()));
                _header.append(Shared->ToStr(HeaderGetChar()));
                _header.append(Shared->ToStr(HeaderGetChar()));

                _FirstTimeGetHeader = false;
            }

            if (Shared->CompareTexts(_header, "WDBC"))
                return dbcFile;
            else if (Shared->CompareTexts(_header, "WCH2"))
                return adbFile;
            else if (Shared->CompareTexts(_header, "WDB2"))
                return db2File;
            else if (Shared->CompareTexts(_header, "BDIW"))
                return wdbitemcacheFile;
            else if (Shared->CompareTexts(_header, "BOMW"))
                return wdbcreaturecacheFile;
            else if (Shared->CompareTexts(_header, "BOGW"))
                return wdbgameobjectcacheFile;
            else if (Shared->CompareTexts(_header, "BDNW"))
                return wdbitemnamecacheFile;
            else if (Shared->CompareTexts(_header, "XTIW"))
                return wdbitemtextcacheFile;
            else if (Shared->CompareTexts(_header, "CPNW"))
                return wdbnpccacheFile;
            else if (Shared->CompareTexts(_header, "XTPW"))
                return wdbpagetextcacheFile;
            else if (Shared->CompareTexts(_header, "TSQW"))
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
        void SetFieldsOffset(vector<enumFieldTypes> FieldTypes)
        {
            _fieldsOffset = new unsigned int[_totalFields];
            _fieldsOffset[0] = 0;
            for (unsigned int i = 1; i < _totalFields; ++i)
            {
                _fieldsOffset[i] = _fieldsOffset[i - 1];
                if (FieldTypes[i - 1] == type_BYTE || FieldTypes[i - 1] == type_UBYTE)
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
        void SetUniqueStringTextsFromStringTable(unsigned long currStringSize)
        {
            string Text = "";
            for (unsigned long currentChar = 1; currentChar < currStringSize; currentChar++)
            {
                char c = static_cast<char>(_stringTable[currentChar]);
                if (c == '\0')
                {
                    SetUniqueStringTexts(Text);
                    Text.clear();
                    continue;
                }

                Text.append(Shared->ToStr(c));
            }
        }
    protected:
        string _fileName;
        enumFileType _fileType;
        vector<enumFieldTypes> _formatedFieldTypes;
        unsigned int _formatedTotalFields;
        unsigned int _formatedRecordSize;

        FILE *_inputFile = NULL;
        unsigned char *_wholeFileData = NULL;

        unsigned long _fileSize = 0;
        unsigned int _headerSize = 0;

        bool _FirstTimeGetHeader = true;
        bool _isASCIIFile = false;
        bool _FirstTimeAksType = true;

        string _header;
        unsigned int _headerOffset = 0;

        unsigned long _dataBytes = 0;
        unsigned long _stringBytes = 0;
        unsigned long _unkBytes = 0;
        unsigned int *_fieldsOffset = NULL;
        unsigned char *_dataTable = NULL;
        unsigned char *_stringTable = NULL;

        map<string, structFileInfo> _ListOfAllFilesToParse;
        map<string, structFileData> _extractedData;
};
#endif