#ifndef _MODULE_CSV_READER_H_
#define _MODULE_CSV_READER_H_

#include "pch.h"
#include "shared.h"

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

class CSV_Reader : public SaveFileInfo
{
    public:
        CSV_Reader(const char *);
        ~CSV_Reader();
        bool LoadCSVFile();
    private:
        bool ExtractFields(string, map<unsigned int, string> &);
        map<unsigned int, string> GetFields(string);
        bool SetFieldTypes(string);
        enumFieldTypes GetFieldType(unsigned int);
        bool CheckFieldsOfEachRecordAndSaveAllData(map<unsigned int, string>);
        bool CheckFieldValue(unsigned int, enumFieldTypes, string, unsigned int);
        const char* GetFieldTypeName(enumFieldTypes);
    protected:
        const char * fileName;
        unsigned int recordSize;
        unsigned int totalRecords;
        unsigned int totalFields;
        map<unsigned int, map<unsigned int, structRecord>> fileData;
        vector<enumFieldTypes> fieldTypes;
};

#endif