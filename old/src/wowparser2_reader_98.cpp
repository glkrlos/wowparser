#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <conio.h>
#include <sstream>
#include <vector>
#include <map>
#include <fstream>
#include "dirent.h"
using namespace std;

class Reader
{
    public:
        Reader();
        ~Reader();
        bool LoadBinary(char *, string, int);
        bool LoadWDB(char *, string, unsigned char *, int);
        bool LoadADB_DBC_DB2(char *, string, unsigned char *, int);
        bool LoadADB_DBC_DB2_predicted(char *, unsigned char *);
        void ExtractBinaryInfo(string fileName);
        template<typename T> string ToStr(T);
        void InitializeBoolFielTypes(int);
    private:
        int totalRecords, totalFields, recordSize, stringSize;
        vector<vector<unsigned char *>> recordData;
        unsigned char *stringData;
        bool *isStringField, *isFloatField, *isIntField, *isByteField, *isBoolField;
        bool isADB, isDB2, isDBC, isWDB, isWDBItem;
};

Reader::Reader()
{
    totalFields = 0;
    totalRecords = 0;
    recordSize = 0;
    stringSize = 0;

    recordData.clear();

    stringData = 0;

    isStringField = 0;
    isFloatField = 0;
    isIntField = 0;
    isByteField = 0;
    isBoolField = 0;
    
    isADB = false;
    isDB2 = false;
    isDBC = false;
    isWDB = false;

    isWDBItem = false;
}

Reader::~Reader()
{
    totalFields = 0;
    totalRecords = 0;
    recordSize = 0;
    stringSize = 0;

    recordData.clear();

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

    isADB = false;
    isDB2 = false;
    isDBC = false;
    isWDB = false;

    isWDBItem = false;
}

void Reader::InitializeBoolFielTypes(int _fields)
{
    isStringField = new bool[_fields];
    isFloatField = new bool[_fields];
    isIntField = new bool[_fields];
    isByteField = new bool[_fields];
    isBoolField = new bool[_fields];

    for (int currentField = 0; currentField < _fields; currentField++)
    {
        isStringField[currentField] = false;
        isFloatField[currentField] = false;
        isIntField[currentField] = false;
        isByteField[currentField] = false;
        isBoolField[currentField] = false;
    }
}

template<typename T> string Reader::ToStr(T i)
{
    ostringstream buffer;

    buffer << i;

    return buffer.str();
}

bool Reader::LoadBinary(char *fileName, string fileFormat, int _recordSize)
{
    FILE *input = fopen(fileName, "rb");
    if(!input)
    {
        printf("ERROR: Can't open file '%s'.\n", fileName);
        return false;
    }

    printf("Loading file: '%s'.\n", fileName);

    fseek(input, 0, SEEK_END);
    long fileSize = ftell(input);

    if (fileSize < 4)
    {
        printf("ERROR: '%s': File size too small.\n", fileName);
        fclose(input);
        return false;
    }

    rewind(input);

    char headerName[4];
    fread(&headerName, 4, 1, input);

    if (headerName[0] == 'W' && headerName[1] == 'C' && headerName[2] == 'H' && headerName[3] == '2')
        isADB = true;
    else if (headerName[0] == 'W' && headerName[1] == 'D' && headerName[2] == 'B' && headerName[3] == '2')
        isDB2 = true;
    else if (headerName[0] == 'W' && headerName[1] == 'D' && headerName[2] == 'B' && headerName[3] == 'C')
        isDBC = true;
    else if (headerName[0] == 'B' && headerName[1] == 'D' && headerName[2] == 'I' && headerName[3] == 'W')        // BDIW itemcache.wdb
    {
        isWDBItem = true;
        isWDB = true;
    }
    else if ((headerName[0] == 'B' && headerName[1] == 'O' && headerName[2] == 'M' && headerName[3] == 'W') ||    // BOMW creaturecache.wdb
             (headerName[0] == 'B' && headerName[1] == 'O' && headerName[2] == 'G' && headerName[3] == 'W') ||    // BOGW gameobjectcache.wdb
             (headerName[0] == 'B' && headerName[1] == 'D' && headerName[2] == 'N' && headerName[3] == 'W') ||    // BDNW itemnamecache.wdb
             (headerName[0] == 'X' && headerName[1] == 'T' && headerName[2] == 'I' && headerName[3] == 'W') ||    // XTIW itemtextcache.wdb
             (headerName[0] == 'C' && headerName[1] == 'P' && headerName[2] == 'N' && headerName[3] == 'W') ||    // CPNW npccache.wdb
             (headerName[0] == 'X' && headerName[1] == 'T' && headerName[2] == 'P' && headerName[3] == 'W') ||    // XTPW pagetextcache.wdb
             (headerName[0] == 'T' && headerName[1] == 'S' && headerName[2] == 'Q' && headerName[3] == 'W')       // TSQW questcache.wdb
            )
        isWDB = true;
    else
    {
        printf("ERROR: '%s': Unknown file type.\n", fileName);
        fclose(input);
        return false;
    }

    // WDB Parse
    if (isWDB)
    {
        // 24 bytes del header + 8 bytes del primer record y su el tamaño del record
        if (fileSize < 32)
        {
            printf("ERROR: '%s': WDB structure is damaged.\n", fileName);
            fclose(input);
            return false;
        }

        // char Header[4];    // actualmente ya esta leido arriba para comprobar el tipo de archivo
        int WDBRevision = 0;
        char WDBLocale[4];
        int WDBMaxRecordSize = 0;
        int unk1 = 0;
        int unk2 = 0;
        // WDBEntry
        // WDBRecordSize

        fread(&WDBRevision, 4, 1, input);
        fread(&WDBLocale, 4, 1, input);
        fread(&WDBMaxRecordSize, 4, 1, input);
        fread(&unk1, 4, 1, input);
        fread(&unk2, 4, 1, input);

        string _tempWDBLocale = ToStr(WDBLocale[3]) + ToStr(WDBLocale[2]) + ToStr(WDBLocale[1]) + ToStr(WDBLocale[0]);

        long WDBDataSize = fileSize - 24;
        unsigned char *WDBData = new unsigned char[WDBDataSize];
        fread(WDBData, WDBDataSize, 1, input);
        fclose(input);
        
        if (LoadWDB(fileName, fileFormat, WDBData, WDBDataSize))
            printf("WDB file loaded: '%s' (Revision: %i, Locale: %s)\n", fileName, WDBRevision, _tempWDBLocale.c_str());
        else
            return false;
    }
    else if (isDBC || isADB || isDB2)
    {
        // tamaño de header en comun para DBC, ADB y DB2
        int headerSize = 4+4+4+4+4;

        if (isDB2)
            headerSize += 4+4+4;

        // Si es DBC o ADB el header tendra que ser de almenos 20 bytes
        // Si es DB2 entonces el header tendra que ser por almenos 32 bytes
        if ( fileSize < headerSize )
        {
            printf("ERROR: '%s': Unknown file format.\n", fileName);
            fclose(input);
            return false;
        }
        
        fread(&totalRecords, 4, 1, input);
        fread(&totalFields, 4, 1, input);
        fread(&recordSize, 4, 1, input);
        fread(&stringSize, 4, 1, input);
    
        if (!totalRecords || !totalFields || !recordSize)
        {
            printf("ERROR: '%s': No records/fields found in file.\n", fileName);
            fclose(input);
            return false;
        }

        long unkBytes = 0;

        if (isDB2)
        {
            int tableHashDB2 = 0;
            int buildDB2 = 0;
            int unk1DB2 = 0;

            fread(&tableHashDB2, 4, 1, input);
            fread(&buildDB2, 4, 1, input);
            fread(&unk1DB2, 4, 1, input);

            if (buildDB2 > 12880)
            {
                headerSize += 4+4+4+4;
                
                if (fileSize < headerSize)
                {
                    printf("ERROR: '%s': Unknown DB2 format.\n", fileName);
                    fclose(input);
                    return false;
                }

                int unk2DB2 = 0;
                int maxIndexDB2 = 0;
                int localesDB2 = 0;
                int unk3DB2 = 0;

                fread(&unk2DB2, 4, 1, input);
                fread(&maxIndexDB2, 4, 1, input);
                fread(&localesDB2, 4, 1, input);
                fread(&unk3DB2, 4, 1, input);

                if (maxIndexDB2 != 0)
                {
                    int diff = maxIndexDB2 - unk2DB2 + 1;
                    // fseek(input, diff * 4 + diff * 2, SEEK_CUR); // diff * 4: an index for rows, diff * 2: a memory allocation bank
                    unkBytes = diff * 4 + diff * 2;
                    unsigned char *unkData = new unsigned char[unkBytes];
                    fread(unkData, unkBytes, 1, input);
                }
            }
        }
        
        long dataBytes = fileSize - headerSize - unkBytes - stringSize;
        long stringBytes = fileSize - headerSize - unkBytes - (totalRecords * recordSize);
        if ((totalRecords < 0) || (totalFields < 0) || (stringSize < 0) ||
            (dataBytes < 0) || (stringBytes < 0) || 
            (dataBytes != (totalRecords * recordSize)) || (stringBytes != stringSize))
        {
            printf("ERROR: '%s': Structure is damaged.\n", fileName);
            fclose(input);
            return false;
        }

        unsigned char *dataData = new unsigned char[dataBytes];
        fread(dataData, dataBytes, 1, input);

        if (stringBytes > 1)
        {
            stringData = new unsigned char[stringSize];
            fread(stringData, stringSize, 1, input);
        }

        fclose(input);

        char *_tempFileType = "";
        if (isDBC)
            _tempFileType = "DBC";
        else if (isADB)
            _tempFileType = "ADB";
        else
            _tempFileType = "DB2";

        if (fileFormat.size())
        {
            if (LoadADB_DBC_DB2(fileName, fileFormat, dataData, _recordSize))
                printf("%s file loaded: '%s'.\n", _tempFileType, fileName);
            else
                return false;
        }
        // Predicted
        else
        {
            if (LoadADB_DBC_DB2_predicted(fileName, dataData))
                printf("Predicted: %s file loaded: '%s'.\n", _tempFileType, fileName);
            else
                return false;
        }
    }

    // por si las dudas
    if (input)
        fclose(input);

    return true;
}

bool Reader::LoadADB_DBC_DB2_predicted(char *fileName, unsigned char *data)
{
    if (recordSize/totalFields != 4)
    {
        if (recordSize % 4 != 0)
        {
            printf("ERROR: '%s': Predicted: Not supported byte packed format.\n", fileName);
            return false;
        }
        totalFields = recordSize / 4;
    }

    // Esta funcion es necesaria, de lo contrario ocasionara crash
    InitializeBoolFielTypes(totalFields);

    int offset = 0;    // contador global usado para saber en que posicion debe leer el siguiente registro

    for (int currentRecord = 0; currentRecord < totalRecords; currentRecord++)
    {
        vector<unsigned char *> fieldData;
        for (int currentField = 0; currentField < totalFields; currentField++)
        {
            int fieldSize = 4;

            fieldData.push_back(data + offset);
            offset += fieldSize;
        }
        recordData.push_back(fieldData);
    }

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
    if (totalRecords > 3)
    {
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

    return true;
}

bool Reader::LoadADB_DBC_DB2(char *fileName, string fileFormat, unsigned char *data, int _recordSize)
{
    if (fileFormat.size() != totalFields)
    {
        printf("ERROR: '%s': Incorrect field format size. Expected '%i' not '%i' fields.\n", fileName, totalFields, fileFormat.size());
        return false;
    }

    if (_recordSize != recordSize)
    {
        printf("ERROR: '%s': Incorrect field format structure, Expected '%i' not %i bytes per record.\n", fileName, recordSize, _recordSize);
        return false;
    }

    // Esta funcion es necesaria, de lo contrario ocasionara crash
    InitializeBoolFielTypes(totalFields);

    // iniciando el tipo de dato
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

    int offset = 0;    // contador global usado para saber en que posicion debe leer el siguiente registro

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

            fieldData.push_back(data + offset);
            offset += fieldSize;
        }
        recordData.push_back(fieldData);
    }

    return true;
}

bool Reader::LoadWDB(char *fileName, string format, unsigned char *data, int dataSize)
{
    // Declaraciones globales en clase, pero totalFields y totalRecords no son definitivas
    // hasta que el final de la funcion
    totalFields = format.size();    // aun le faltan 2, pero eso se hace hasta abajo de esta funcion
    totalRecords = 0;
    stringSize = 0;

    // Esta funcion es necesaria, de lo contrario ocasionara crash
    InitializeBoolFielTypes(totalFields + 1);

    long orignaldataSize = dataSize + 24;

    bool isFirstRecord = true;
    int offset = 0;    // contador global usado para saber en que posicion debe leer el siguiente registro

    // repetir desde aqui
    while (true)
    {
        int entry = 0;
        int recordSize = 0;

        if ((dataSize -= 8) >= 0)
        {
            entry = *reinterpret_cast<int *>(data + offset);
            offset += 4;
            recordSize = *reinterpret_cast<int *>(data + offset);
            offset += 4;

            if (isFirstRecord && (!entry || !recordSize))
            {
                printf("ERROR: '%s': No records found.\n", fileName);
                return false;
            }
            else if (!isFirstRecord && (!entry || !recordSize))
                break;

            isFirstRecord = false;

            if ((dataSize -= recordSize) >= 0)
            {
                vector<unsigned char *> fieldData;

                // entry, se le restan 8 al offset, debido a que ya se le sumaron 8
                fieldData.push_back(data + (offset - 8));
                isIntField[0] = true;
                // StatsCount caso especial para itemcache.wdb
                bool CountItemStats = false;
                bool isCompletedStats = true;
                bool neverRepeat = true;
                int StatsCount = 0;
                int StatsRemaining = 20;

                for (int currentField = 0; currentField < totalFields; currentField++)
                {
                    if (CountItemStats && neverRepeat)
                    {
                        if (!StatsRemaining)
                        {
                            isCompletedStats = true;
                            neverRepeat = false;
                        }
                        else
                        {
                            isCompletedStats = false;
                            if (StatsCount)
                            {
                                isIntField[currentField + 1] = true;
                                fieldData.push_back(data + offset);
                                offset += 4;
                                recordSize -= 4;
                                StatsCount -= 1;
                                StatsRemaining -= 1;
                            }
                            else
                            {
                                isIntField[currentField + 1] = true;
                                unsigned char *_tempiii = new unsigned char[sizeof(int)];
                                _tempiii[0] = 0;
                                _tempiii[1] = 0;
                                _tempiii[2] = 0;
                                _tempiii[3] = 0;
                                fieldData.push_back(_tempiii);
                                StatsRemaining -= 1;
                            }
                        }
                    }

                    if (isCompletedStats)
                    {
                        switch(format[currentField])
                        {
                            case 'b':    // byte
                            case 'X':    // unk byte
                                //printf("%c, %i\n", format[currentField], *reinterpret_cast<char *>(data + offset));
                                isByteField[currentField + 1] = true;
                                fieldData.push_back(data + offset);
                                offset += 1;
                                recordSize -= 1;
                                break;
                            case 's':
                            {
                                string _tempString = reinterpret_cast<char *>(data + offset);
                                //printf("%c, %s\n", format[currentField], _tempString.c_str());
                                isStringField[currentField + 1] = true;
                                fieldData.push_back(data + offset);
                                offset += _tempString.size() + 1;
                                recordSize -= _tempString.size() + 1;
                                break;
                            }
                            case 'd':    // int
                            case 'n':    // int
                            case 'x':    // unk int
                            case 'i':    // int
                                //printf("%c, %i\n", format[currentField], *reinterpret_cast<int *>(data + offset));
                                isIntField[currentField + 1] = true;
                                if (isWDBItem && (format[currentField] == 'x') && !CountItemStats)
                                {
                                    StatsCount = *reinterpret_cast<int *>(data + offset);
                                    StatsCount *= 2;
                                    CountItemStats = true;
                                }
                                fieldData.push_back(data + offset);
                                offset += 4;
                                recordSize -= 4;
                                break;
                            case 'f':    // float
                                //printf("%c, %f\n", format[currentField], *reinterpret_cast<float *>(data + offset));
                                isFloatField[currentField + 1] = true;
                                fieldData.push_back(data + offset);
                                offset += 4;
                                recordSize -= 4;
                                break;
                        } // switch(format[currentField])
                    } // if (isCompletedStats)

                    if ((recordSize > 0) && ((currentField + 1) >= totalFields))
                    {
                        printf("ERROR: '%s': You must read '%i' bytes more per record.\n", fileName, recordSize);
                        return false;
                    }
                    else if ((recordSize < 0) && ((currentField + 1) >= totalFields))
                    {
                        printf("ERROR: '%s': Exceeded record size by '%i' bytes.\n", fileName, recordSize * -1);
                        return false;
                    }
                } // for (unsigned int currentField = 0; currentField < totalFields; currentField++)

                recordData.push_back(fieldData);
            } // if ((dataSize -= recordSize) >= 0)
            else
            {
                printf("ERROR: '%s': Corrupted WDB file.\n", fileName);
                return false;
            }
        } // if ((dataSize -= 8) >= 0)
        else
        {
            printf("ERROR: '%s': Unexpected End of file in WDB, expected file size '%li'.\n", fileName, orignaldataSize-dataSize);
            return false;
        }
    } // while (true)

    // Del total de fields de formato solo se agrega uno mas que es el entry
    totalFields += 1;
    totalRecords = recordData.size();

    return true;
}

void Reader::ExtractBinaryInfo(string fileName)
{
    string outputFileName = fileName + ".csv";
    FILE *output = fopen(outputFileName.c_str(), "w");
    if(!output)
    {
        printf("ERROR: File cannot be created '%s'.\n", outputFileName.c_str());
        return;
    }

    for (int currentField = 0; currentField < totalFields; currentField++)
    {
        if (isStringField[currentField])
            fprintf(output, "string");
        else if (isFloatField[currentField])
            fprintf(output, "float");
        else if (isByteField[currentField])
            fprintf(output, "byte");
        else if (isIntField[currentField] || isBoolField[currentField])
            fprintf(output, "int");

        if (currentField+1 < totalFields)
            fprintf(output, ",");
    }
    fprintf(output, "\n");

    for (int currentRecord = 0; currentRecord < totalRecords; currentRecord++)
    {
        for (int currentField = 0; currentField < totalFields; currentField++)
        {
            if (!isWDB && (stringSize > 1) && isStringField[currentField])
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
                    fprintf(output, "%s", outText.c_str());
                }
            }
            else if (isWDB && isStringField[currentField])
            {
                string _tempText = reinterpret_cast<char *>(recordData[currentRecord][currentField]);
                int value = _tempText.size();
                if (value)
                {
                    string outText = "\"";
                    for (int x = 0; x < value; x++)
                    {
                        if (!_tempText[x])
                            break;

                        if (_tempText[x] == '"')
                            outText.append("\"");

                        if (_tempText[x] == '\r')
                        {
                            outText.append("\\r");
                            continue;
                        }

                        if (_tempText[x] == '\n')
                        {
                            outText.append("\\n");
                            continue;
                        }

                        outText.append(ToStr(_tempText[x]));
                    }
                    outText.append("\"");
                    fprintf(output, "%s", outText.c_str());
                }
            }
            else if (isFloatField[currentField])
                fprintf(output, "%f", *reinterpret_cast<float *>(recordData[currentRecord][currentField]));
            else if (isByteField[currentField])
                fprintf(output, "%d", *reinterpret_cast<char *>(recordData[currentRecord][currentField]));
            else if (isIntField[currentField] || isBoolField[currentField])
                fprintf(output, "%i", *reinterpret_cast<int *>(recordData[currentRecord][currentField]));

            if (currentField+1 < totalFields)
                fprintf(output, ",");
        }
        fprintf(output, "\n");
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
            if (cReader.LoadBinary((char*)it->first.c_str(), it->second.Structure, it->second.recordSize))
                cReader.ExtractBinaryInfo(it->first);
        }
    }

    printf("\nWorld of Warcraft Data Parser to CSV\n");
    printf("Copyright(c) 2012 Carlos Ramzuel - itsunited.com - Tlaxcala, Mexico.\n");
    printf("Version 2.0 Build 98 (October 30 2011)\n");

    _getch();
    return 0;
}