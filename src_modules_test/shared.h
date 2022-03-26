#ifndef _SHARED_H_
#define _SHARED_H_

#include "pch.h"

enum enumFieldTypes
{
    type_NONE   = 0,
    type_STRING = 1,
    type_FLOAT  = 2,
    type_BYTE   = 3,
    type_UBYTE  = 4,
    type_INT    = 5,
    type_UINT   = 6,
    type_BOOL   = 7,
};

struct structRecord
{
    enumFieldTypes fieldType;
    string fieldValue;
};

struct structFileInfo
{
    const char* fileName;
    unsigned int recordSize;
    unsigned int totalRecords;
    unsigned int totalFields;
    unsigned int stringSize;
    string stringTexts;
};

template<typename T> string ToStr(T i)
{
    ostringstream buffer;

    buffer << i;

    return buffer.str();
}

class Fields
{
    public:
        Fields(structRecord field, unsigned int fieldID) : _field(field), _fieldID(fieldID) {}
        unsigned int GetID() { return _fieldID; }
        unsigned int GetType() { return _field.fieldType; }
        const char* GetString() { return _field.fieldValue.c_str(); }
        int GetInt() { return atoi(_field.fieldValue.c_str()); }
        float GetFloat() { return float(atof(_field.fieldValue.c_str())); }
    protected:
        structRecord _field;
        unsigned int _fieldID;
};

class Records
{
    public:
        Records(map<unsigned int, structRecord> record, unsigned int recordID) :
            _record(record), _recordID(recordID), _fieldID(-1) {}
        unsigned int GetID() { return _recordID; }
        bool NextField()
        {
            _fieldID++;

            if (_fieldID < 0 || _fieldID >= int(_record.size()))
                return false;

            return true;
        }
        Fields GetField() { return Fields(_record[_fieldID], _fieldID); }

        // para multiarray unsafe
        // Fields operator[] (int FieldID) { return Fields(_record[FieldID], FieldID); }
    private:
        map<unsigned int, structRecord> _record;
        int _fieldID;
        unsigned int _recordID;
};

class FileInfo
{
    public:
        FileInfo(structFileInfo structFileInfo) : _structFileInfo(structFileInfo) {}
        const char *GetFileName() { return _structFileInfo.fileName; }
        unsigned int GetRecordSize() { return _structFileInfo.recordSize; }
        unsigned int GetTotalFields() { return _structFileInfo.totalFields; }
        unsigned int GetTotalRecords() { return _structFileInfo.totalRecords; }
        unsigned int GetStringSize() { return _structFileInfo.stringSize; }
        const char *GetStringTexts() { return _structFileInfo.stringTexts.c_str(); }
        unsigned int GetStringTextsSize() { return _structFileInfo.stringTexts.size(); }
    protected:
        structFileInfo _structFileInfo;
};

class FileData : public FileInfo
{
    public:
        FileData(structFileInfo _structFileInfo, map<unsigned int, map<unsigned int, structRecord>> fileData) :
            FileInfo(_structFileInfo), _fileData(fileData), _recordID(-1) {}
        bool NextRecord()
        {
            _recordID++;

            if (_recordID < 0 || _recordID >= int(_fileData.size()))
                return false;

            return true;
        }
        Records GetRecord() { return Records(_fileData[_recordID], _recordID); }

        // para multiarray unsafe
        // Records operator[] (unsigned int RecordID) { return Records(_fileData[RecordID], RecordID); }
    private:
        map<unsigned int, map<unsigned int, structRecord>> _fileData;
        int _recordID;
};

class SaveFileInfo
{
    public:
        SaveFileInfo()
        {
            _structFileInfo.stringTexts = '\0';
            _structFileInfo.stringSize = 1;
        }
        void SetFileName(const char *fileName) { _structFileInfo.fileName = fileName; }
        void SetRecordSize(unsigned int recordSize) { _structFileInfo.recordSize = recordSize; }
        void SetTotalFields(unsigned int totalFields) { _structFileInfo.totalFields = totalFields; }
        void SetTotalRecords(unsigned int totalRecords) { _structFileInfo.totalRecords = totalRecords; }
        void SetUniqueStringTexts(string Text)
        {
            if (!Text.empty() && !GetUniqueStringTextsPosition(Text))
            {
                unsigned int currentStringPos = _structFileInfo.stringTexts.size();
                _structFileInfo.stringTexts.append(Text + '\0');
                _structFileInfo.stringSize += Text.size() + 1;
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
        void GetUniqueStringTexts()
        {
            string Text = "";
            for (unsigned int currentChar = 1; currentChar < _structFileInfo.stringTexts.size(); currentChar++)
            {
                if (_structFileInfo.stringTexts[currentChar] == '\0')
                {
                    SetUniqueStringTexts(Text);
                    Text.clear();
                    continue;
                }

                Text += _structFileInfo.stringTexts[currentChar];
            }
        }
    protected:
        structFileInfo _structFileInfo;
        map<string, unsigned int> _uniqueStringTexts;
};
#endif