#ifndef _SHARED_H_
#define _SHARED_H_

#include "pch.h"
#include <algorithm>

enum enumFieldTypes
{
    type_NONE   = 0,
    type_STRING = 1,
    type_FLOAT  = 2,
    type_BYTE   = 3,
    type_INT    = 4,
    type_UINT   = 5,
    type_BOOL   = 6,
    type_UBYTE  = 7,
};

enum enumFileType
{
    unkFile = 0,
    dbcFile = 1,
    db2File = 2,
    adbFile = 3,
    wdbFile = 4,
    csvFile = 5,
    totalFileTypes = 6,

    wdbitemcacheFile = 7,
    wdbcreaturecacheFile = 8,
    wdbgameobjectcacheFile = 9,
    wdbitemnamecacheFile = 10,
    wdbitemtextcacheFile = 11,
    wdbnpccacheFile = 12,
    wdbpagetextcacheFile = 13,
    wdbquestcacheFile = 14,
};

struct structField
{
    structField()
    {
        ID = 0;
        Type = type_NONE;
        StringValue = 0;
        UIntValue = 0;
        IntValue = 0;
        BoolValue = false;
        ByteValue = 0;
        UByteValue = 0;
        FloatValue = 0.0f;
    }
    unsigned int ID;
    enumFieldTypes Type;
    unsigned int StringValue;
    unsigned int UIntValue;
    int IntValue;
    unsigned int BoolValue;
    char ByteValue;
    unsigned char UByteValue;
    float FloatValue;
};

struct structRecord
{
    unsigned int ID;
    vector<structField> Field;
};

struct structFileData
{
    vector<structRecord> Record;
};

struct outputFormat
{
    outputFormat()
    {
        ToCSV = false;
        ToDBC = false;
        ToSQL = false;
        isSetToCSV = false;
        isSetToDBC = false;
        isSetToSQL = false;
    }
    bool ToCSV;
    bool ToDBC;
    bool ToSQL;
    bool isSetToCSV;
    bool isSetToDBC;
    bool isSetToSQL;
};

struct structXMLFileInfo
{
    public:
        structXMLFileInfo()
        {
            FileName.clear();
            Type = unkFile;
            Structure.clear();
            isRecursivelySearched = false;
            isSearchedByExtension = false;
            XMLFileID = 0;
            FormatedFieldTypes.clear();
            FormatedTotalFields = 0;
            FormatedRecordSize = 0;
        }

        string FileName;
        enumFileType Type;
        string Structure;
        bool isRecursivelySearched = false;
        bool isSearchedByExtension = false;
        unsigned int XMLFileID = 0;
        vector<enumFieldTypes> FormatedFieldTypes;
        unsigned int FormatedTotalFields;
        unsigned int FormatedRecordSize;
        outputFormat outputFormats;
};

class SaveFileInfo
{
    public:
        SaveFileInfo()
        {
            _recordSize = 0;
            _totalFields = 0;
            _totalRecords = 0;

            _fieldTypes.clear();
            _savedData.clear();

            _stringSize = 1;
            _stringTexts = '\0';
            _uniqueStringTexts.clear();
        }
        ~SaveFileInfo()
        {
            _recordSize = 0;
            _totalFields = 0;
            _totalRecords = 0;

            _fieldTypes.clear();
            _savedData.clear();

            _stringSize = 0;
            _stringTexts.clear();
            _uniqueStringTexts.clear();
        }
        unsigned int GetTotalFields() { return _totalFields; }
        unsigned int GetTotalRecords() { return _totalRecords; }
        unsigned int GetRecordSize() { return _recordSize; }
        unsigned int GetStringSize() { return _stringSize; }
        vector<enumFieldTypes> GetFieldTypes() { return _fieldTypes; }
        string GetStringTexts() { return _stringTexts; }
        map<string, vector<unsigned int>> GetUniqueStringTexts() { return _uniqueStringTexts; }
        map<string, structFileData> GetExtractedData() { return _savedData; }
        void SetUniqueStringTexts(string Text)
        {
            if (!Text.empty() && !GetUniqueTextPosition(Text))
            {
                unsigned int TextPosition = _stringTexts.size();
                _stringTexts.append(Text + '\0');
                _stringSize += Text.size() + 1;

                vector<unsigned int> VectorForTextPosition;
                VectorForTextPosition.push_back(TextPosition);
                _uniqueStringTexts.insert(pair<string, vector<unsigned int>>(Text, VectorForTextPosition));
            }
        }
        unsigned int GetUniqueTextPosition(string Text)
        {
            if (!Text.empty())
            {
                auto FindText = _uniqueStringTexts.find(Text);
                if (FindText != _uniqueStringTexts.end())
                    return *min_element(FindText->second.begin(), FindText->second.end());
            }

            return 0;
        }
        void SetUniqueStringTexts(string Text, unsigned int TextPosition)
        {
            if (Text.empty())
                return;

            auto FindText = _uniqueStringTexts.find(Text);
            if (FindText != _uniqueStringTexts.end())
            {
                auto FindPosition = find(FindText->second.begin(), FindText->second.end(), TextPosition);
                if (FindPosition != FindText->second.end())
                {
                    Text.clear();
                    return;
                }

                FindText->second.push_back(TextPosition);
                Text.clear();
                return;
            }

            vector<unsigned int> VectorForTextPosition;
            VectorForTextPosition.push_back(TextPosition);
            _uniqueStringTexts.insert(pair<string, vector<unsigned int>>(Text, VectorForTextPosition));
            Text.clear();
            return;
        }
    protected:
        unsigned int _recordSize;
        unsigned int _totalFields;
        unsigned int _totalRecords;

        vector<enumFieldTypes> _fieldTypes;
        map<string, structFileData> _savedData;

        unsigned int _stringSize;
        string _stringTexts;
        map<string, vector<unsigned int>> _uniqueStringTexts;
};
class cShared
{
    public:
        static cShared* Instance()
        {
            static cShared instance;
            return &instance;
        }
        template <typename T> string ToStr(T i)
        {
            ostringstream buffer;

            buffer << i;

            return buffer.str();
        }
        void ToLowerCase(string &text)
        {
            transform(text.begin(), text.end(), text.begin(), ::tolower);
        }
        bool CompareTexts(string txt1, string txt2)
        {
            return !txt1.compare(txt2);
        }
        const char *GetFileExtensionByFileType(enumFileType eFT)
        {
            switch (eFT)
            {
                case dbcFile: return "dbc";
                case db2File: return "db2";
                case adbFile: return "adb";
                case wdbFile: return "wdb";
                case csvFile: return "csv";
                default: return "Unknown";
            }
        }
};

#define Shared cShared::Instance()

// __FUNCSIG__ para imprimir el nombre de la funcion completa y localizar algun fallo si algo no sale bien

/*
class classSingleton
{
    public:
        static classSingleton& Instance()
        {
            static classSingleton instance;
            return instance;
        }
    private:
    protected:
};

#define TestClassOfSingleton classSingleton::Instance()
*/
#endif
