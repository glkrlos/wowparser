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
    protected:
        int totalRecords, totalFields, recordSize, stringSize;
        string stringData;
        map<int, float> *floatData;
        map<int, char> *byteData;
        map<int, int> *intData;
};

Writer::Writer()
{
    totalRecords = 0;
    totalFields = 0;
    recordSize = 0;
    stringSize = 1;

    stringData = '\0';

    floatData = 0;
    byteData = 0;
    intData = 0;
}

Writer::~Writer()
{
    totalRecords = 0;
    totalFields = 0;
    recordSize = 0;
    stringSize = 0;

    stringData.empty();

    if (floatData)
        delete[] floatData;

    if (byteData)
        delete[] byteData;

    if (intData)
        delete[] intData;
}

template<typename T> string Writer::ToStr(T i)
{
    ostringstream buffer;

    buffer << i;

    return buffer.str();
}

bool Writer::LoadText(string filename)
{
    ifstream input(filename.c_str(), ifstream::in);
    if (!input.is_open())
    {
        printf("ERROR: Can't open file '%s'.\n", filename.c_str());
        return false;
    }

    printf("Loading file '%s'.\n", filename.c_str());

    map<int,int> fieldType;
    // fieldType
    {
        string firstLine = "";
        getline(input, firstLine, '\n');
        int firstLineSize = firstLine.size();
        if (firstLineSize < 3)
        {
            printf("ERROR: Unknown CSV format in file '%s'.\n", filename.c_str());
            printf("First line must contain field types (int, float, byte or string).\n");
            input.close();
            return false;
        }

        int prevPos = 0, lastPos = 0, count = 0;
        for (;;)
        {
            lastPos = firstLine.find(",", prevPos);
            string tempFieldName = firstLine.substr(prevPos, (lastPos < 0) ? (firstLine.size() - prevPos) : (lastPos - prevPos));
            if (!tempFieldName.compare("string"))
            {
                fieldType.insert(pair<int, int>(count, mSTRING));
                recordSize += 4;
                count++;
            }
            else if (!tempFieldName.compare("float"))
            {
                fieldType.insert(pair<int, int>(count, mFLOAT));
                recordSize += 4;
                count++;
            }
            else if (!tempFieldName.compare("byte"))
            {
                fieldType.insert(pair<int, int>(count, mBYTE));
                recordSize += 1;
                count++;
            }
            else if (!tempFieldName.compare("int"))
            {
                fieldType.insert(pair<int, int>(count, mINT));
                recordSize += 4;
                count++;
            }
            else if (tempFieldName.empty())
            {
                printf("ERROR: Empty Field Name in CSV file '%s'.\n", filename.c_str());
                input.close();
                return false;
            }
            else
            {
                printf("ERROR: Unknown Field Type '%s' in CSV file '%s'.\n", tempFieldName.c_str(), filename.c_str());
                input.close();
                return false;
            }

            prevPos = lastPos + 1;
            if (lastPos < 0)
                break;
        }
    }

    // totalFields
    totalFields = fieldType.size();
    if (!totalFields)
    {
        printf("ERROR: No fields found or unknown format in CSV file '%s'.\n", filename.c_str());
        input.close();
        return false;
    }

    // totalRecords
    {
        long long filePosAferFirstLine = input.tellg();
        string tempLine = "";
        while (getline(input, tempLine, '\n'))
            totalRecords++;

        if (!totalRecords)
        {
            printf("ERROR: No records found or unknown format in CSV file '%s'.\n", filename.c_str());
            input.close();
            return false;
        }
        input.clear();
        input.seekg(filePosAferFirstLine, ios_base::beg);
    }

    // Record Data
    floatData = new map<int, float>[totalRecords];
    byteData = new map<int, char>[totalRecords];
    intData = new map<int, int>[totalRecords];
    map<string, int> uniqueStringTexts;

    int counterLine = 0;
    for (int currentRecord = 0; currentRecord < totalRecords; currentRecord++)
    {
        string currentLine = "";
        getline(input, currentLine, '\n');
        counterLine++;
        int prevPos = 0, lastPos = 0, counterField = 0;
        for (map<int,int>::iterator int_it = fieldType.begin(); int_it != fieldType.end(); int_it++)
        {
            if ((lastPos < 0) && (counterField != totalFields))
            {
                printf("ERROR: CSV file '%s': Needs more fields at line '%i'.\n", filename.c_str(), counterLine+1);
                printf("This file must contain only '%i' fields per record.\n", totalFields);
                input.close();
                return false;
            }

            string stringValue = "";
            if (int_it->second == mSTRING)
            {
                counterField++;
                stringValue = currentLine.substr(prevPos, currentLine.size());

                if (stringValue[0] == '"')
                {
                    string _TEMP = "";
                    unsigned int currentChr = 0;
                    int countercomma = 1;
                    for (currentChr = 1; currentChr < stringValue.size(); currentChr++)
                    {
                        if (stringValue[currentChr] == '"')
                        {
                            if (stringValue[currentChr+1] == '"')
                            {
                                currentChr++;
                                _TEMP.append("\"");
                                continue;
                            }

                            if ((stringValue[currentChr+1] != ',') && (currentChr+1 < stringValue.size()))
                            {
                                printf("%i, %i\n", currentChr+1, stringValue.size());
                                printf("ERROR: CSV file '%s': Unexpected end of string in field '%i' at line '%i'.\n", filename.c_str(), counterField, counterLine+1);
                                input.close();
                                return false;
                            }

                            countercomma--;
                            break;
                        }
                        if (stringValue[currentChr] == '\\')
                        {
                            if (stringValue[currentChr+1] == 'n')
                            {
                                currentChr++;
                                _TEMP.append("\n");
                                continue;
                            }
                            else if (stringValue[currentChr+1] == 'r')
                            {
                                currentChr++;
                                _TEMP.append("\r");
                                continue;
                            }
                        }

                        _TEMP.append(ToStr(stringValue[currentChr]));
                    }

                    if (countercomma)
                    {
                        printf("ERROR: CSV file '%s': Missing \" in the end of string in field '%i' at line '%i'.\n", filename.c_str(), counterField, counterLine+1);
                        input.close();
                        return false;
                    }
                    stringValue = _TEMP;

                    if (counterField == 1 && totalFields > 1)
                        lastPos += currentChr + 1;
                    else
                        lastPos += currentChr + 2;
                }
                else if (stringValue[0] == ',' || stringValue.empty())
                {
                    stringValue = "";
                    lastPos += 1;
                }
                else
                {
                    printf("ERROR: CSV file '%s': Broken string format '%c' at line '%i'.\n", filename.c_str(), stringValue[0], counterLine+1);
                    printf("If is an empty string use a comma, if not empty put the text between \".\n");
                    input.close();
                    return false;
                }

                if (!stringValue.empty())
                {
                    map<string, int>::iterator string_it = uniqueStringTexts.find(stringValue);
                    if (string_it != uniqueStringTexts.end())
                        intData[currentRecord].insert(pair<int, int>(int_it->first, string_it->second));
                    else
                    {
                        int currentStringPos = stringData.size();
                        stringData.append(stringValue + '\0');
                        uniqueStringTexts.insert(pair<string, int>(stringValue, currentStringPos));
                        intData[currentRecord].insert(pair<int, int>(int_it->first, currentStringPos));
                    }
                }
                else
                    intData[currentRecord].insert(pair<int, int>(int_it->first, 0));
            }
            else
            {
                counterField++;

                if (prevPos > (int)currentLine.size())
                {
                    printf("ERROR: CSV file '%s': Missing comma after field '%i' at line '%i'.\n", filename.c_str(), counterField, counterLine+1);
                    printf("This file must contain only '%i' fields per record.\n", totalFields);
                    input.close();
                    return false;
                }

                lastPos = currentLine.find(",", prevPos);
                stringValue = currentLine.substr(prevPos, (lastPos < 0) ? (currentLine.size() - prevPos) : (lastPos - prevPos));
                if (stringValue.empty())
                {
                    printf("ERROR: CSV file '%s': Empty int/float/byte field at line '%i'\n", filename.c_str(), counterLine+1);
                    printf("The integer, byte or float field must have at least a value of 0.\n");
                    input.close();
                    return false;
                }

                if (int_it->second == mFLOAT)
                    floatData[currentRecord].insert(pair<int, float>(int_it->first, (float)atof(stringValue.c_str())));
                else if (int_it->second == mBYTE)
                {
                    int _tempByte = atoi(stringValue.c_str());
                    if (_tempByte < -128 || _tempByte > 255)
                        printf("WARNING: CSV file '%s': Overloaded value '%i' for byte in field '%i' at line '%i'.\n", filename.c_str(), _tempByte, counterField, counterLine+1);

                    byteData[currentRecord].insert(pair<int, char>(int_it->first, atoi(stringValue.c_str())));
                }
                else if (int_it->second == mINT)
                    intData[currentRecord].insert(pair<int, int>(int_it->first, atoi(stringValue.c_str())));
            }

            if ((counterField >= totalFields) && (lastPos != -1))
            {
                string _tempStringValue = currentLine.substr(lastPos-1, currentLine.size());
                if (!_tempStringValue.empty())
                    printf("WARNING: CSV file '%s': Contains more fields than expected at line '%i'. (ignoring).\n", filename.c_str(), counterLine+1);
            }

            prevPos = lastPos + 1;        // contador global no remover
        }
    }

    stringSize = stringData.size();

    input.close();

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
        printf("ERROR: Can't create file '%'.\n", filename.c_str());
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
            if (floatData[currentRecord].find(currentField) != floatData[currentRecord].end())
                fwrite(&floatData[currentRecord][currentField], 4, 1, output);
            else if (byteData[currentRecord].find(currentField) != byteData[currentRecord].end())
                fwrite(&byteData[currentRecord][currentField], 1, 1, output);
            else if (intData[currentRecord].find(currentField) != intData[currentRecord].end())
                fwrite(&intData[currentRecord][currentField], 4, 1, output);
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
    printf("Copyright(c) 2011 Carlos Ramzuel - Tlaxcala, Mexico.\n");
    printf("Version 2.0 Build 85 (March 28 2011)\n");
    _getch();
    return 0;
}
