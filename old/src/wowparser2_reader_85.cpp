#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <sstream>
#include <map>
#include <fstream>
#include "dirent.h"
#include <conio.h>

using namespace std;

class Reader
{
    public:
        Reader();
        ~Reader();
        template<typename T> string ToStr(T);
        bool LoadBinary(string, string, int);
        void LoadBinaryFieldTypesPredicted();
        void ExtractBinaryInfo(string);
    private:
        int totalRecords, totalFields, recordSize, stringSize;
        vector<vector<unsigned char *>> recordData;
        unsigned char *stringData;
        bool *isStringField, *isFloatField, *isIntField, *isByteField, *isBoolField, isPredicted;
};

Reader::Reader()
{
    totalFields = 0;
    totalRecords = 0;
    recordSize = 0;
    stringSize = 0;
    // ADB, DB2 unicamente
    // int unk1 = 0;
    // int coreBuild = 0;
    // int unixTime = 0;        // solo en ADB -> WCH2
    // int firstEntry = 0;
    // int lastEntry = 0;
    // int locale = 0;
    // int unk2 = 0;

    recordData.empty();

    stringData = 0;

    isStringField = 0;
    isFloatField = 0;
    isIntField = 0;
    isByteField = 0;
    isBoolField = 0;

    isPredicted = false;
}

Reader::~Reader()
{
    totalFields = 0;
    totalRecords = 0;
    recordSize = 0;
    stringSize = 0;

    recordData.empty();

    if (stringData)
        delete[] stringData;

    if (isStringField)
        delete[] isStringField;

    if (isFloatField)
        delete[] isFloatField;

    if (isIntField)
        delete[] isIntField;

    if (isByteField)
        delete[] isByteField;

    if (isBoolField)
        delete[] isBoolField;

    isPredicted = false;
}

template<typename T> string Reader::ToStr(T i)
{
    ostringstream buffer;

    buffer << i;

    return buffer.str();
}

bool Reader::LoadBinary(string fileName = "", string fileFormat = "", int _recordSize = 0)
{
    FILE *input;
    fopen_s(&input, fileName.c_str(), "rb");
    if(!input)
    {
        printf("ERROR: Can't open file '%s'.\n", fileName.c_str());
        return false;
    }

    printf("Loading file: '%s'.\n", fileName.c_str());

    fseek(input, 0, SEEK_END);
    long fileSize = ftell(input);

    if (fileSize < 20)
    {
        printf("ERROR: Unknown file format '%s'.\n", fileName.c_str());
        fclose(input);
        return false;
    }

    rewind(input);

    char headerName[4];
    fread(&headerName, 4, 1, input);

    bool isADB = false, isDB2 = false, isDBC = false;

    char *typeFormat = "";
    if (headerName[0] == 'W' && headerName[1] == 'C' && headerName[2] == 'H' && headerName[3] == '2')
    {
        isADB = true;
        typeFormat = "ADB";
    }
    else if (headerName[0] == 'W' && headerName[1] == 'D' && headerName[2] == 'B' && headerName[3] == '2')
    {
        isDB2 = true;
        typeFormat = "DB2";
    }
    else if (headerName[0] == 'W' && headerName[1] == 'D' && headerName[2] == 'B' && headerName[3] == 'C')
    {
        isDBC = true;
        typeFormat = "DBC";
    }
    else
    {
        printf("ERROR: Unknown file type '%s'.\n", fileName.c_str());
        fclose(input);
        return false;
    }

    fread(&totalRecords, 4, 1, input);
    fread(&totalFields, 4, 1, input);
    fread(&recordSize, 4, 1, input);
    fread(&stringSize, 4, 1, input);

    if (!totalRecords || !totalFields || !recordSize)
    {
        printf("ERROR: No records/fields found in %s file '%s'.\n", typeFormat, fileName.c_str());
        fclose(input);
        return false;
    }

    int headerSize = 20;
    long unkBytes = fileSize - headerSize - (totalRecords * recordSize) - stringSize;
    long dataBytes = fileSize - headerSize - unkBytes - stringSize;
    long stringBytes = fileSize - headerSize - unkBytes - (totalRecords * recordSize);
    if ((totalRecords < 0) || (totalFields < 0) || (stringSize < 0) ||
        (dataBytes < 0) || (stringBytes < 0) || 
        (dataBytes != (totalRecords * recordSize)) || (stringBytes != stringSize))
    {
        printf("ERROR: Structure damaged or unknown format in %s file '%s'.\n", typeFormat, fileName.c_str());
        fclose(input);
        return false;
    }

    if (fileFormat.size() && fileFormat.size() != totalFields)
    {
        printf("ERROR: Incorrect field format size for file '%s'.\n", fileName.c_str());
        printf("Your Format Fields: %i, Expexted Fields: %i.\n", fileFormat.size(), totalFields);
        fclose(input);
        return false;
    }

    if (unkBytes)
    {
        unsigned char *unkData = new unsigned char[unkBytes];
        fread(unkData, unkBytes, 1, input);
    }

    if (fileFormat.size() && _recordSize)
    {
        if (_recordSize != recordSize)
        {
            printf("ERROR: Check field format structure, expected record size: %i, your record format: %i for file '%s'.\n", recordSize, _recordSize, fileName.c_str());
            fclose(input);
            return false;
        }

        isStringField = new bool[totalFields];
        isFloatField = new bool[totalFields];
        isIntField = new bool[totalFields];
        isByteField = new bool[totalFields];
        isBoolField = new bool[totalFields];

        for (int currentField = 0; currentField < totalFields; currentField++)
        {
            isStringField[currentField] = false;
            isFloatField[currentField] = false;
            isIntField[currentField] = false;
            isByteField[currentField] = false;
            isBoolField[currentField] = false;
        }

        for (int currentField = 0; currentField < totalFields; currentField++)
        {
            switch (fileFormat[currentField])
            {
                case 'b':    // byte
                case 'X':    // unk byte
                    isByteField[currentField] = true;
                    break;
                case 's':    // string
                    isStringField[currentField] = true;
                    break;
                case 'd':    // int
                case 'n':    // int
                case 'x':    // unk int
                case 'i':    // int
                    isIntField[currentField] = true;
                    break;
                case 'f':    // float
                    isFloatField[currentField] = true;
                    break;
            }
        }

        for (int currentRecord = 0; currentRecord < totalRecords; currentRecord++)
        {
            vector<unsigned char *> fieldData;
            for (int currentField = 0; currentField < totalFields; currentField++)
            {
                int fieldSize = 0;

                if (isByteField[currentField])
                    fieldSize = 1;
                else
                    fieldSize = 4;
                
                unsigned char *currentFieldData = new unsigned char[fieldSize];
                fread(currentFieldData, fieldSize, 1, input);
                fieldData.push_back(currentFieldData);
            }
            recordData.push_back(fieldData);
        }
    }
    else
    {
        if (recordSize/totalFields != 4)
        {
            if (recordSize % 4 != 0)
            {
                printf("ERROR: Predicted -> Not supported byte packed format for file '%s'.\n", fileName.c_str());
                fclose(input);
                return false;
            }

            totalFields = recordSize / 4;
        }

        isStringField = new bool[totalFields];
        isFloatField = new bool[totalFields];
        isIntField = new bool[totalFields];
        isByteField = new bool[totalFields];
        isBoolField = new bool[totalFields];

        for (int currentField = 0; currentField < totalFields; currentField++)
        {
            isStringField[currentField] = false;
            isFloatField[currentField] = false;
            isIntField[currentField] = false;
            isByteField[currentField] = false;
            isBoolField[currentField] = false;
        }

        isPredicted = true;
    
        for (int currentRecord = 0; currentRecord < totalRecords; currentRecord++)
        {
            vector<unsigned char *> fieldData;
            for (int currentField = 0; currentField < totalFields; currentField++)
            {
                unsigned char *currentFieldData = new unsigned char[recordSize/totalFields];
                fread(currentFieldData, recordSize/totalFields, 1, input);
                fieldData.push_back(currentFieldData);
            }
            recordData.push_back(fieldData);
        }
    }

    if (stringSize > 1)
    {
        stringData = new unsigned char[stringSize];
        fread(stringData, stringSize, 1, input);
    }

    fclose(input);

    printf("%s file loaded '%s'.\n", typeFormat, fileName.c_str());

    if (isPredicted)
        LoadBinaryFieldTypesPredicted();

    return true;
}

void Reader::LoadBinaryFieldTypesPredicted()
{
    // Float Field System
    for (int currentField = 0; currentField < totalFields; currentField++)
    {
        int counterfloat = 0, counterint = 0;

        for (int currentRecord = 0; currentRecord < totalRecords; currentRecord++)
        {
            int value = *reinterpret_cast<int *>(recordData[currentRecord][currentField]);
            if (value)
            {
                float floatValue = *reinterpret_cast<float *>(recordData[currentRecord][currentField]);
                string floatString = ToStr(floatValue);
                int isFloat1 = floatString.find("e");
                int isFloat2 = floatString.find("#");
                if ((isFloat1 == -1) && (isFloat2 == -1))
                    counterfloat++;
                else
                    counterint++;
            }
        }

        if (counterfloat > counterint)
            isFloatField[currentField] = true;
        else
            isFloatField[currentField] = false;
    }

    // Bool Field System
    for (int currentField = 0; currentField < totalFields; currentField++)
    {
        if (!isFloatField[currentField])
        {
            bool isBool = true;

            for (int currentRecord = 0; currentRecord < totalRecords; currentRecord++)
            {
                int value = *reinterpret_cast<int *>(recordData[currentRecord][currentField]);
                if (value < 0 || value > 1)
                {
                    isBool = false;
                    break;
                }
            }

            if (isBool)
                isBoolField[currentField] = true;
        }
    }

    // String Field System
    if (stringSize > 1)
    {
        for (int currentField = 0; currentField < totalFields; currentField++)
        {
            if (!isFloatField[currentField] && !isBoolField[currentField])
            {
                for (int currentRecord = 0; currentRecord < totalRecords; currentRecord++)
                {
                    int value = *reinterpret_cast<int *>(recordData[currentRecord][currentField]);
                    if ((value < 0) || (value >= stringSize))
                    {
                        isStringField[currentField] = false;
                        break;
                    }
                    else if ((value > 0) && (stringData[value-1]))
                    {
                        isStringField[currentField] = false;
                        break;
                    }
                    else if ((value > 0) && !stringData[value-1])
                        isStringField[currentField] = true;
                }
            }
        }
    }

    // Int Field System
    for (int currentField = 0; currentField < totalFields; currentField++)
        if (!isFloatField[currentField] && !isStringField[currentField] && !isBoolField[currentField])
            isIntField[currentField] = true;

    // Byte Field false siempre para predicted
    for (int currentField = 0; currentField < totalFields; currentField++)
        isByteField[currentField] = false;
}

void Reader::ExtractBinaryInfo(string fileName)
{
    string outputFileName = fileName + ".csv";
    FILE *output;
    fopen_s(&output, outputFileName.c_str(), "w");
    if(!output)
    {
        printf("ERROR: File cannot be created '%s'.\n", outputFileName.c_str());
        return;
    }

    for (int currentField = 0; currentField < totalFields; currentField++)
    {
        if (isStringField[currentField])
            fprintf_s(output, "string");
        else if (isFloatField[currentField])
            fprintf_s(output, "float");
        else if (isByteField[currentField])
            fprintf_s(output, "byte");
        else if (isIntField[currentField] || isBoolField[currentField])
            fprintf_s(output, "int");

        if (currentField+1 < totalFields)
            fprintf_s(output, ",");
    }
    fprintf_s(output, "\n");

    for (int currentRecord = 0; currentRecord < totalRecords; currentRecord++)
    {
        for (int currentField = 0; currentField < totalFields; currentField++)
        {
            if ((stringSize > 1) && isStringField[currentField])
            {
                int value = *reinterpret_cast<int *>(recordData[currentRecord][currentField]);
                if (value)
                {
                    string outText = "\"";
                    for (int x = value; x < stringSize; x++)
                    {
                        if (!stringData[x])
                            break;

                        if (stringData[x] == '"')
                            outText.append("\"");

                        if (stringData[x] == '\r')
                        {
                            outText.append("\\r");
                            continue;
                        }

                        if (stringData[x] == '\n')
                        {
                            outText.append("\\n");
                            continue;
                        }

                        outText.append(ToStr(stringData[x]));
                    }
                    outText.append("\"");
                    fprintf_s(output, "%s", outText.c_str());
                }
            }
            else if (isFloatField[currentField])
                fprintf_s(output, "%f", *reinterpret_cast<float *>(recordData[currentRecord][currentField]));
            else if (isByteField[currentField])
                fprintf_s(output, "%d", *reinterpret_cast<char *>(recordData[currentRecord][currentField]));
            else if (isIntField[currentField] || isBoolField[currentField])
                fprintf_s(output, "%i", *reinterpret_cast<int *>(recordData[currentRecord][currentField]));

            if (currentField+1 < totalFields)
                fprintf_s(output, ",");
        }
        fprintf_s(output, "\n");
    }

    fclose(output);

    printf("CSV file created: '%s'.\n", outputFileName.c_str());
}

// Main
struct FileStructure
{
    string Structure;
    int recordSize;
};

map<string, FileStructure> mFileNames, mTempFileNames;
bool isConfig = true;

void AddFileStructureToList()
{
    char *filename = "WoWParserReader.cfg";
    ifstream input(filename, ifstream::in);
    if (!input.is_open())
    {
        printf("Warning: Can't open configuration file '%s' for field structure.\nOnly Byte Fields can't be predicted.\n\n", filename);
        printf("Adding all DBC, DB2, ADB files in current directory (recursive) to list.\n");
        isConfig = false;
    }
    else
    {
        string Line = "";
        int countLine = 0;
        while (getline(input, Line))
        {
            countLine++;
            if (Line.empty() || Line[0] == ' ' || Line[0] == '#')
                continue;

            int pos = Line.find('\t', 0);
            string tempFileName = Line.substr(0, pos);
            string tempFileStructure = "";
            int recordSize = 0;
            
            int isDirectory = tempFileName.rfind("/");
            if (isDirectory+1 == tempFileName.size())
            {
                printf("ERROR: %s is a directory not a file (skiping at line %i)\n", tempFileName.c_str(), countLine);
                continue;
            }

            int plus = tempFileName.rfind("*.");
            if (plus != -1)
            {
                string plusFileName = tempFileName.substr(plus, tempFileName.size());
                string secondPlusFileName = plusFileName.substr(2, plusFileName.size());
                int secondplus = secondPlusFileName.find("*");
                int sizeplustFileName = plusFileName.size();
                if (sizeplustFileName < 3 || secondplus != -1)
                {
                    printf("Warning: Invalid use *.* or *. needs file extension, for example: *.dbc or ./dirname/*.dbc (ignoring at line %i).\n", countLine);
                    continue;
                }
            }
            else if (pos == -1 || ((pos != -1) && Line.substr(pos+1, Line.size()).empty()))
                printf("Warning: No structure found for file '%s' at line %i (Field types will be predicted).\n", Line.substr(0, (pos == -1) ? Line.size() : pos).c_str(), countLine);
            else if (pos != -1)
            {
                tempFileStructure = Line.substr(pos+1, Line.size());
            
                bool validStructure = true;
                for (unsigned int x = 0; x < tempFileStructure.size(); x++)
                {
                    switch(tempFileStructure[x])
                    {
                        case 'b':    // byte
                        case 'X':    // unk byte
                            recordSize += 1;
                            break;
                        case 's':    // string
                        case 'd':    // int
                        case 'n':    // int
                        case 'x':    // unk int
                        case 'i':    // int
                        case 'f':    // float
                            recordSize += 4;
                            break;
                        default:
                            printf("ERROR: Invalid structure character '%c' for file '%s' at line: %i (Skiping)\n", tempFileStructure[x], tempFileName.c_str(), countLine);
                            x = tempFileStructure.size();
                            validStructure = false;
                            break;
                    }
                }

                if (!validStructure)
                    continue;
            }

            map<string, FileStructure>::iterator it = mTempFileNames.find(tempFileName);
            if (it != mTempFileNames.end())
            {
                printf("WARNING: File name already exists in structure: %s at line: %i (Ignoring)\n", tempFileName.c_str(), countLine);
                continue;
            }
            else
            {
                FileStructure sFileStructure;
                sFileStructure.Structure = tempFileStructure;
                sFileStructure.recordSize = recordSize;
                mTempFileNames.insert(pair<string, FileStructure>(tempFileName, sFileStructure));
            }
        }
    }
}

void AddFilesToList(string directory = ".", string filename = "", string structure = "", int recordsize = 0, bool recursive = false, string fileExt = "")
{
    DIR *dir;
    struct dirent *ent;

    if ((dir=opendir(directory.c_str())) != NULL)
    {
        while((ent=readdir(dir)) != NULL)
        {
            if (!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, ".."))
                continue;

            if (!recursive)
            {
                string dirName = directory + ent->d_name;

                if (ent->d_type == DT_REG)
                {
                    FileStructure sFileStructure;
                    sFileStructure.Structure = structure;
                    sFileStructure.recordSize = recordsize;

                    if (!fileExt.empty())
                    {
                        string _tempFileName = ent->d_name;
                        int _tempPosExt = _tempFileName.rfind(".");
                        if (_tempPosExt != -1)
                        {
                            string _tempExt = _tempFileName.substr(_tempPosExt, _tempFileName.size());
                            if (!strcmp(_tempExt.c_str(), fileExt.c_str()))
                                mFileNames.insert(pair<string, FileStructure>(dirName, sFileStructure));
                        }
                    }
                    else if (ent->d_name == filename)
                    {
                        map<string, FileStructure>::iterator it = mFileNames.find(dirName);
                        if (it != mFileNames.end())
                        {
                            it->second.Structure = structure;
                            it->second.recordSize = recordsize;
                        }
                        else
                            mFileNames.insert(pair<string, FileStructure>(dirName, sFileStructure));
                    }
                }
            }
            else
            {
                string dirName = directory + "/" + ent->d_name;

                if (ent->d_type == DT_REG)
                {
                    FileStructure sFileStructure;
                    sFileStructure.Structure = structure;
                    sFileStructure.recordSize = recordsize;

                    if (!fileExt.empty())
                    {
                        string _tempFileName = ent->d_name;
                        int _tempPosExt = _tempFileName.rfind(".");
                        if (_tempPosExt != -1)
                        {
                            string _tempExt = _tempFileName.substr(_tempPosExt, _tempFileName.size());
                            if (!strcmp(_tempExt.c_str(), fileExt.c_str()))
                                mFileNames.insert(pair<string, FileStructure>(dirName, sFileStructure));
                        }
                    }
                    else if (ent->d_name == filename)
                    {
                        map<string, FileStructure>::iterator it = mFileNames.find(dirName);
                        if (it != mFileNames.end())
                        {
                            it->second.Structure = structure;
                            it->second.recordSize = recordsize;
                        }
                        else
                            mFileNames.insert(pair<string, FileStructure>(dirName, sFileStructure));
                    }
                }
                else
                    AddFilesToList(dirName, filename, structure, recordsize, true, fileExt);
            }
        }
        closedir(dir);
    }
}

int main(int argc, char *arg[])
{
    AddFileStructureToList();

    if (isConfig)
    {
        for (map<string,FileStructure>::iterator it = mTempFileNames.begin(); it != mTempFileNames.end(); it++)
        {
            int pos = it->first.rfind("/");
            int rpos = it->first.rfind("*.");
            // archivos en directorio especifico
            if (pos != -1)
            {
                string path = it->first.substr(0, pos+1);
                if (rpos != -1)
                {
                    string fileext = it->first.substr(rpos+1, it->first.size());
                    AddFilesToList(path, "", "", 0, false, fileext);
                }
                else
                {
                    string file = it->first.substr(pos+1, it->first.size());
                    AddFilesToList(path, file, it->second.Structure, it->second.recordSize, false);
                }
            }
            // archivos recursivos
            else
            {
                if (rpos != -1)
                {
                    string fileext = it->first.substr(rpos+1, it->first.size());
                    AddFilesToList(".", "", "", 0, true, fileext);
                }
                else
                    AddFilesToList(".", it->first, it->second.Structure, it->second.recordSize, true);
            }
        }
    }
    else
    {
        AddFilesToList(".", "", "", 0, true, ".dbc");
        AddFilesToList(".", "", "", 0, true, ".db2");
        AddFilesToList(".", "", "", 0, true, ".adb");
    }

    if (isConfig && !mFileNames.size())
        printf("No file specified or not files found in configuration file.\n");
    if (!isConfig && !mFileNames.size())
        printf("No ADB, DB2, DBC files found in current directory.\n");
    else
    {
        for (map<string,FileStructure>::iterator it = mFileNames.begin(); it != mFileNames.end(); it++)
        {
            Reader cReader;
            if (cReader.LoadBinary(it->first, it->second.Structure, it->second.recordSize))
                cReader.ExtractBinaryInfo(it->first);
        }
    }

    printf("\nWorld of Warcraft Data Parser to CSV\n");
    printf("Copyright(c) 2011 Carlos Ramzuel - Tlaxcala, Mexico.\n");
    printf("Version 2.0 Build 85 (March 28 2011)\n");
    _getch();
    return 0;
}