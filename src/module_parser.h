#ifndef _MODULE_PARSER_H_
#define _MODULE_PARSER_H_

#include "pch.h"
#include "shared.h"

class module_parser
{
    public:
        module_parser(structFile sFile)
        {
            _sFile = sFile;
            _stringTexts = '\0';
        }
        ~module_parser()
        {
            _header.clear();

            if (_fileData)
            {
                delete _fileData;
                _fileData = NULL;
            }

            if (_inputFile) fclose(_inputFile);
        }
        bool Load();
        void ParseFile();
    private:
        bool ParseBinaryFile();
        bool ParseCSVFile();
        bool CheckStructure();
        bool PredictFieldTypes();
        bool IsPreFormatted() { return !_sFile.FormatedFieldTypes.empty(); }
        const char *GetFileName() { return _sFile.FileName.c_str(); }
        enumFileType GetFileType() { return _sFile.Type; }
        bool FileIsASCII()
        {
            if (_FirstTimeAksType)
            {
                _FirstTimeAksType = false;

                bool binary = false;
                for (unsigned long x = 0; x < _fileSize; x++)
                {
                    if (static_cast<char>(_fileData[x]) == '\n' || static_cast<char>(_fileData[x]) == '\r')
                        continue;

                    if (!isprint(static_cast<char>(_fileData[x])))
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
        bool NullPoniterToData() { return !_fileData; }
        char HeaderGetChar() { return static_cast<char>(_fileData[HeaderGetOffSet(1)]); }
        unsigned int HeaderGetUInt() { return *reinterpret_cast<unsigned int*>(_fileData + HeaderGetOffSet(4)); }
        unsigned int HeaderGetOffSet(size_t size) { return (_headerOffset + size) <= _fileSize ? (_headerOffset += size) - size : 0; }

        void SetFieldTypesToNONE()
        {
            _sFile.FormatedFieldTypes.clear();
            for (unsigned int x = 0; x < _totalFields; x++)
                _sFile.FormatedFieldTypes.push_back(type_NONE);
        }
        void SetFieldsOffset()
        {
            _fieldsOffset = new unsigned int[_totalFields];
            _fieldsOffset[0] = 0;
            for (unsigned int i = 1; i < _totalFields; ++i)
            {
                _fieldsOffset[i] = _fieldsOffset[i - 1];
                if (_sFile.FormatedFieldTypes[i - 1] == type_BYTE || _sFile.FormatedFieldTypes[i - 1] == type_UBYTE)
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

        bool CreateCSVFile();
        bool CreateDBCFile();
        void SetUniqueStringTexts(string Text)
        {
            if (!Text.empty() && !GetUniqueStringTextsPosition(Text))
            {
                unsigned int currentStringPos = _stringTexts.size();
                _stringTexts.append(Text + '\0');
                //_stringSize += Text.size() + 1;
                _uniqueStringTexts.insert(pair<string, unsigned int>(Text, currentStringPos));
            }
        }
        unsigned int GetUniqueStringTextsPosition(string Text)
        {
            if (!Text.empty())
            {
                map<string, unsigned int>::iterator it = _uniqueStringTexts.find(Text);
                if (it != _uniqueStringTexts.end())
                    return (it->second);
            }

            return 0;
        }
    protected:
        structFile _sFile;

        FILE *_inputFile = NULL;
        unsigned char *_fileData = NULL;

        unsigned long _fileSize = 0;
        unsigned int _headerSize = 0;
        unsigned int _totalRecords = 0;
        unsigned int _totalFields = 0;
        unsigned int _recordSize = 0;
        unsigned int _stringSize = 0;
        unsigned int _countFloatFields = 0;
        unsigned int _countStringFields = 0;
        unsigned int _countBoolFields = 0;
        unsigned int _countIntFields = 0;
        unsigned int _countUIntFields = 0;

        bool _FirstTimeGetHeader = true;
        bool _isASCIIFile = false;
        bool _FirstTimeAksType = true;

        string _header;
        unsigned int _headerOffset = 0;

        long _dataBytes = 0;
        long _stringBytes = 0;
        long _unkBytes = 0;
        unsigned int *_fieldsOffset = NULL;
        unsigned char *_dataTable = NULL;
        unsigned char *_stringTable = NULL;

        map<string, unsigned int> _uniqueStringTexts;
        string _stringTexts;
};
#endif