#ifndef _MODULE_CSV_READER_H_
#define _MODULE_CSV_READER_H_

#include "pch.h"
#include "shared.h"
#include "log.h"

struct structField
{
    unsigned int ID;
    enumFieldTypes Type;
    string Value;
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

class SaveFileInfo
{
    public:
        SaveFileInfo()
        {
            _stringTexts = '\0';
            _stringSize = 1;
        }
        void SetUniqueStringTexts(string Text)
        {
            if (!Text.empty() && !GetUniqueStringTextsPosition(Text))
            {
                unsigned int currentStringPos = _stringTexts.size();
                _stringTexts.append(Text + '\0');
                _stringSize += Text.size() + 1;
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
            for (unsigned int currentChar = 1; currentChar < _stringTexts.size(); currentChar++)
            {
                if (_stringTexts[currentChar] == '\0')
                {
                    SetUniqueStringTexts(Text);
                    Text.clear();
                    continue;
                }

                Text += _stringTexts[currentChar];
            }
        }
    protected:
        unsigned int _stringSize;
        string _stringTexts;
        map<string, unsigned int> _uniqueStringTexts;
};

class CSV_Reader : public SaveFileInfo
{
    public:
        CSV_Reader(const char *);
        ~CSV_Reader();
        bool LoadCSVFile();
        bool ParseFile();
        void PrintResults();
        void CreateDBCFile();
    private:
        // map<unsigned int, string> GetFields(string);
        bool ExtractFields(string, map<unsigned int, string> &);

        bool SetFieldTypes(string);
        enumFieldTypes GetFieldType(unsigned int);
        const char* GetFieldTypeName(enumFieldTypes);

        bool CheckFieldValue(unsigned int, enumFieldTypes, string, unsigned int);
        bool CheckFieldsOfEachRecordAndSaveAllData();
    protected:
        const char * _fileName = NULL;
        unsigned int _recordSize = 0;
        unsigned int _totalFields = 0;
        unsigned int _totalRecords = 0;
        map<string, structFileData> _fileData;
        vector<enumFieldTypes> _fieldTypes;
        map<unsigned int, string> _mapRecordsData;

        unsigned int _countFloatFields = 0;
        unsigned int _countStringFields = 0;
        unsigned int _countBoolFields = 0;
        unsigned int _countByteFields = 0;
        unsigned int _countUByteFields = 0;
        unsigned int _countIntFields = 0;
        unsigned int _countUIntFields = 0;
};

#endif