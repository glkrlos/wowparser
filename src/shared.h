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

            _stringTexts = '\0';
            _stringSize = 1;
        }
        ~SaveFileInfo()
        {
            _recordSize = 0;
            _totalFields = 0;
            _totalRecords = 0;

            _fieldTypes.clear();
            _savedData.clear();

            _stringTexts.clear();
            _stringSize = 0;
        }
        unsigned int GetTotalTotalFields() { return _totalFields; }
        unsigned int GetTotalTotalRecords() { return _totalRecords; }
        unsigned int GetTotalRecordSize() { return _recordSize; }
        unsigned int GetStringSize() { return _stringSize; }
        vector<enumFieldTypes> GetFieldTypes() { return _fieldTypes; }
        string GetStringTexts() { return _stringTexts; }
        map<string, unsigned int> GetUniqueStringTexts() { return _uniqueStringTexts; }
        map<string, structFileData> GetExtractedData() { return _savedData; }

        void SetUniqueStringTexts(string Text)
        {
            if (!Text.empty() && !GetUniqueTextPosition(Text))
            {
                unsigned int currentStringPos = _stringTexts.size();
                _stringTexts.append(Text + '\0');
                _stringSize += Text.size() + 1;
                _uniqueStringTexts.insert(pair<string, unsigned int>(Text, currentStringPos));
            }
        }
        unsigned int GetUniqueTextPosition(string Text)
        {
            if (!Text.empty())
            {
                auto it = _uniqueStringTexts.find(Text);
                if (it != _uniqueStringTexts.end())
                    return (it->second);
            }

            return 0;
        }
        string GetUniqueTextFromPosition(unsigned int position)
        {
            if (!position)
                return "";

            string texttoreturn = "";
            for (auto current = _uniqueStringTexts.begin(); current != _uniqueStringTexts.end(); current++)
            {
                if (current->second == position)
                {
                    texttoreturn = current->first;
                    break;
                }
            }

            return texttoreturn;
        }
    protected:
        unsigned int _recordSize;
        unsigned int _totalFields;
        unsigned int _totalRecords;
        vector<enumFieldTypes> _fieldTypes;
        map<string, structFileData> _savedData;

        unsigned int _stringSize;
        string _stringTexts;
        map<string, unsigned int> _uniqueStringTexts;
};

template <typename T> class CSingleton
{
    public:
        static T* Instance()
        {
            if (!m_instance.get())
                m_instance = auto_ptr<T>(new T);

            return m_instance.get();
        };
    protected:
        //CSingleton();
        //~CSingleton();
    private:
        //CSingleton(CSingleton const&);
        //CSingleton& operator = (CSingleton const*);
        static auto_ptr<T> m_instance;
};

template <typename T> auto_ptr<T> CSingleton<T>::m_instance;

class cShared
{
    public:
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

#define Shared CSingleton<cShared>::Instance()

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
