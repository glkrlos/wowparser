#ifndef _MODULE_DBC_WRITER_H_
#define _MODULE_DBC_WRITER_H_

#include "pch.h"
#include "shared.h"
#include "module_parser.h"

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

class DBC_Writer
{
    public:
        DBC_Writer(FileData *);
        ~DBC_Writer();
        void CreateDBC();
    private:
        FileData *_file;
};

#endif