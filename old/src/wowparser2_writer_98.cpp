#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <map>
#include <fstream>
#include <sstream>
#include <vector>
#include <conio.h>
#include "dirent.h"

using namespace std;

struct GenericHeader
{
    char headerName[4];
    int totalRecords;
    int totalFields;
    int recordSize;
    int stringSize;
};

enum fielTypeNamesEnum
{
    mSTRING    = 0,
    mFLOAT    = 1,
    mBYTE    = 2,
    mINT    = 4,
};

class Writer
{
    public:
        Writer();
        ~Writer();
        bool LoadText(string);
        void CreateDBC(string);
    private:
        template<typename T> string ToStr(T i);
        bool NextField(string &, string &);
        bool NextStringField(string &, string &);
    protected:
        int totalRecords, totalFields, recordSize, stringSize;
        vector<vector<unsigned char *>> recordData;
        string stringData;
        map<int, int> *intField;
        map<int, float> *floatField;
        map<int, char> *byteField;
        string filename;
        int counterLine, counterField;
};

Writer::Writer()
{
    totalRecords = 0;
    totalFields = 0;
    recordSize = 0;
    stringSize = 1;

    recordData.clear();

    stringData = '\0';

    intField = 0;
    floatField = 0;
    byteField = 0;

    filename.clear();

    counterLine = 1;
    counterField = 0;
}

Writer::~Writer()
{
    totalRecords = 0;
    totalFields = 0;
    recordSize = 0;
    stringSize = 0;

    recordData.clear();

    stringData.clear();

    if (intField)
        delete[] intField;

    if (floatField)
        delete[] floatField;

    if (byteField)
        delete[] byteField;

    intField = 0;
    floatField = 0;
    byteField = 0;

    filename.clear();

    counterLine = 1;
    counterField = 0;
}

template<typename T> string Writer::ToStr(T i)
{
    ostringstream buffer;

    buffer << i;

    return buffer.str();
}

bool Writer::NextField(string &fieldName, string &fieldValue)
{
    if (fieldName.empty())
        return false;

    int pos = fieldName.find(",");

    if (pos != -1)
    {
        fieldValue = fieldName.substr(0, pos);
        int tempSize = fieldName.size();
        if ((pos+1) >= tempSize)
            fieldName.erase(0, pos);
        else
            fieldName.erase(0, pos+1);
    }
    else
    {
        fieldValue = fieldName.substr(0, fieldName.size());
        fieldName.erase(0, fieldName.size());
    }

    return true;
}

bool Writer::NextStringField(string &fieldName, string &fieldValue)
{
    if (fieldName.empty() || (fieldName[0] == ','))
    {
        if (!fieldName.empty())
            fieldName.erase(0, 1);
        return true;
    }
    else if (fieldName[0] == '"')
    {
        int countercomma = 1;
        unsigned int currentChr = 0;
        for (currentChr = 1; currentChr < fieldName.size(); currentChr++)
        {
            if (fieldName[currentChr] == '"')
            {
                if (fieldName[currentChr+1] == '"')
                {
                    currentChr++;
                    fieldValue.append("\"");
                    continue;
                }

                if ((fieldName[currentChr+1] != ',') && (currentChr+1 < fieldName.size()))
                {
                    printf("ERROR: '%s': Unexpected end of string in field '%i' at line '%i'.\n", filename.c_str(), counterField, counterLine);
                    return false;
                }

                countercomma--;
                break;
            }
            else if (fieldName[currentChr] == '\\')
            {
                if (fieldName[currentChr+1] == 'n')
                {
                    currentChr++;
                    fieldValue.append("\n");
                    continue;
                }
                else if (fieldName[currentChr+1] == 'r')
                {
                    currentChr++;
                    fieldValue.append("\r");
                    continue;
                }
            }
            fieldValue.append(ToStr(fieldName[currentChr]));
        }
    
        if (countercomma)
        {
            printf("ERROR: '%s': Missing \" in the end of string in field '%i' at line '%i'.\n", filename.c_str(), counterField, counterLine);
            return false;
        }

        currentChr++;    // se suma la primera coma al tamaño total del string

        if ((currentChr + 1) < fieldName.size())
            fieldName.erase(0, currentChr+1);
        else
            fieldName.erase(0, currentChr);
    }
    else
    {
        printf("ERROR: '%s': Broken string format '%c' at line '%i'.\n", filename.c_str(), fieldName[0], counterLine);
        printf("If is an empty string leave field empty, if is not empty put the text between \".\n");
        return false;
    }

    return true;
}

bool Writer::LoadText(string _filename)
{
    filename = _filename;
    ifstream input(filename.c_str(), ifstream::in);
    if (!input.is_open())
    {
        printf("ERROR: Can't open file '%s'.\n", filename.c_str());
        return false;
    }

    printf("Loading file '%s'.\n", filename.c_str());

    // fieldType
    map<int,int> fieldType;
    string fieldNames = "";
    getline(input, fieldNames, '\n');
    if (fieldNames.empty() || fieldNames.size() < 3)
    {
        printf("ERROR: '%s': First line must contain field types (int,float,byte,string).\n", filename.c_str());
        input.close();
        return false;
    }

    string currentFieldName = "";
    while (NextField(fieldNames, currentFieldName))
    {
        if (!currentFieldName.compare("string"))
        {
            fieldType.insert(pair<int, int>(totalFields, mSTRING));
            recordSize += 4;
            totalFields++;
        }
        else if (!currentFieldName.compare("float"))
        {
            fieldType.insert(pair<int, int>(totalFields, mFLOAT));
            recordSize += 4;
            totalFields++;
        }
        else if (!currentFieldName.compare("byte"))
        {
            fieldType.insert(pair<int, int>(totalFields, mBYTE));
            recordSize += 1;
            totalFields++;
        }
        else if (!currentFieldName.compare("int"))
        {
            fieldType.insert(pair<int, int>(totalFields, mINT));
            recordSize += 4;
            totalFields++;
        }
        else if (currentFieldName.empty())
        {
            printf("ERROR: '%s': Name of field '%i' is empty.\n", filename.c_str(), totalFields+1);
            input.close();
            return false;
        }
        else
        {
            printf("ERROR: '%s': Unknown field type '%s'.\n", filename.c_str(), currentFieldName.c_str());
            input.close();
            return false;
        }
    }

    // checar por si las dudas
    if (totalFields != fieldType.size())
    {
        printf("ERROR: '%s': WTF This should never happen.\n", filename.c_str());
        input.close();
        return false;
    }

    // totalRecords
    vector<string> _temprecordData;
    string tempLine = "";
    while (getline(input, tempLine, '\n'))
        _temprecordData.push_back(tempLine);

    input.close();

    totalRecords = _temprecordData.size();
    if (!totalRecords)
    {
        printf("ERROR: '%s': No records found.\n", filename.c_str());
        input.close();
        return false;
    }

    // iniciando tipos de fields
    intField = new map<int, int>[totalRecords];
    floatField = new map<int, float>[totalRecords];
    byteField = new map<int, char>[totalRecords];

    // mapa para evitar repetir el string al final del dbc
    map<string, int> uniqueStringTexts;

    // extrayendo informacion de _temprecordData
    for (int currentRecord = 0; currentRecord < totalRecords; currentRecord++)
    {
        counterField = 0;
        counterLine++;
        for (map<int,int>::iterator int_it = fieldType.begin(); int_it != fieldType.end(); int_it++, counterField++)
        {
            if (_temprecordData[currentRecord].empty() && (counterField != totalFields))
            {
                bool haveErrorIsEmpty = false;

                if ( (int_it->second != mSTRING) || ((int_it->second == mSTRING) && (counterField+1 != totalFields)) )
                    haveErrorIsEmpty = true;

                if (haveErrorIsEmpty)
                {
                    // originalmente estaban solo estas siguientes dos lineas en esta comprobacion
                    // if (_temprecordData[currentRecord].empty() && (counterField != totalFields))
                    printf("ERROR: '%s': Expected '%i' fields not '%i' at line '%i'.\n", filename.c_str(), totalFields, (int_it->second == mSTRING) ? counterField+1 : counterField, counterLine);
                    return false;
                }
            }

            if (int_it->second == mSTRING)
            {
                string stringValue = "";
                if (!NextStringField(_temprecordData[currentRecord], stringValue))
                    return false;

                if (!stringValue.empty())
                {
                    map<string, int>::iterator string_it = uniqueStringTexts.find(stringValue);
                    if (string_it != uniqueStringTexts.end())
                        intField[currentRecord].insert(pair<int, int>(int_it->first, string_it->second));
                    else
                    {
                        int currentStringPos = stringData.size();
                        stringData.append(stringValue + '\0');
                        uniqueStringTexts.insert(pair<string, int>(stringValue, currentStringPos));
                        intField[currentRecord].insert(pair<int, int>(int_it->first, currentStringPos));
                    }
                }
                else
                    intField[currentRecord].insert(pair<int, int>(int_it->first, 0));
            }
            else
            {
                string numericValue = "";
                NextField(_temprecordData[currentRecord], numericValue);
                if (numericValue.empty())
                {
                    printf("ERROR: '%s': Empty int/float/byte field '%i' at line '%i'.\n", filename.c_str(), counterField+1, counterLine);
                    return false;
                }

                if (int_it->second == mFLOAT)
                {
                    floatField[currentRecord].insert(pair<int, float>(int_it->first, (float)atof(numericValue.c_str())));
                }
                else if (int_it->second == mBYTE)
                {
                    byteField[currentRecord].insert(pair<int, char>(int_it->first, atoi(numericValue.c_str())));
                }
                else if (int_it->second == mINT)
                {
                    intField[currentRecord].insert(pair<int, int>(int_it->first, atoi(numericValue.c_str())));
                }
            }

            if (!_temprecordData[currentRecord].empty() && ((counterField + 1) >= totalFields))
            {
                printf("WARNING: '%s': Contains more fields than expected at line '%i'. (ignoring).\n", filename.c_str(), counterLine);
                break;
            }
        }
    }

    // tamaño del string
    stringSize = stringData.size();

    printf("CSV file loaded '%s'.\n", filename.c_str());

    return true;
}

void Writer::CreateDBC(string filename)
{
    FILE *output;
    string outFileName = filename + ".dbc";
    fopen_s(&output, outFileName.c_str(), "wb");
    if(!output)
    {
        printf("ERROR: '%s': Can't create file.\n", filename.c_str());
        return;
    }

    GenericHeader DBCHeader;
    DBCHeader.headerName[0] = 'W';
    DBCHeader.headerName[1] = 'D';
    DBCHeader.headerName[2] = 'B';
    DBCHeader.headerName[3] = 'C';
    DBCHeader.totalRecords = totalRecords;
    DBCHeader.totalFields = totalFields;
    DBCHeader.recordSize = recordSize;
    DBCHeader.stringSize = stringSize;
    fwrite(&DBCHeader, sizeof(GenericHeader), 1, output);

    for (int currentRecord = 0; currentRecord < totalRecords; currentRecord++)
    {
        for (int currentField = 0; currentField < totalFields; currentField++)
        {
            if (intField[currentRecord].find(currentField) != intField[currentRecord].end())
                fwrite(&intField[currentRecord][currentField], 4, 1, output);
            else if (floatField[currentRecord].find(currentField) != floatField[currentRecord].end())
                fwrite(&floatField[currentRecord][currentField], 4, 1, output);
            else if (byteField[currentRecord].find(currentField) != byteField[currentRecord].end())
                fwrite(&byteField[currentRecord][currentField], 1, 1, output);
        }
    }

    fwrite(reinterpret_cast<const char *>(stringData.c_str()), stringData.size(), 1, output);

    fclose(output);

    printf("DBC file created '%s'.\n", outFileName.c_str());
}

vector<string> FileNames;

void AddFilesToList(string directory = ".")
{
    DIR *dir;
    struct dirent *ent;

    if ((dir=opendir(directory.c_str())) != NULL)
    {
        while((ent=readdir(dir)) != NULL)
        {
            if (!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, ".."))
                continue;

            string dirName = directory + "/" + ent->d_name;
            if (ent->d_type == DT_REG)
            {
                int posExt = dirName.rfind(".");
                string fileExt = dirName.substr(posExt, dirName.size());
                if (!strcmp(fileExt.c_str(), ".csv"))
                    FileNames.push_back(dirName);
            }
            else
                AddFilesToList(dirName);
        }
        closedir(dir);
    }
}

int main(int argc, char *arg[])
{
    AddFilesToList();
    if (!FileNames.size())
        printf("No CSV files found in current directory.\n");
    else
    {
        for (unsigned int currentFile = 0; currentFile < FileNames.size(); currentFile++)
        {
            Writer mWriter;
            if (mWriter.LoadText(FileNames[currentFile]))
                mWriter.CreateDBC(FileNames[currentFile]);
        }
    }

    printf("\nCSV Parser to World of Warcraft DBC file.\n");
    printf("Copyright(c) 2012 Carlos Ramzuel - itsunited.com - Tlaxcala, Mexico.\n");
    printf("Version 2.0 Build 98 (October 30 2011)\n");
    _getch();
    return 0;
}
