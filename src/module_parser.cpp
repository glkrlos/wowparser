#include "module_parser.h"

bool module_parser::Load()
{
    _inputFile = fopen(GetFileName(), "rb");
    if (!_inputFile)
    {
        Log->WriteLog("ERROR: Can't open file '%s'.\n", GetFileName());
        return false;
    }

    Log->WriteLog("Reading file '%s'... ", GetFileName());

    fseek(_inputFile, 0, SEEK_END);
    _fileSize = ftell(_inputFile);

    if (!_fileSize)
    {
        Log->WriteLogNoTime("FAILED: Empty File.\n");
        Log->WriteLog("\n");
        return false;
    }

    rewind(_inputFile);

    _fileData = new unsigned char[_fileSize];
    if (fread(_fileData, _fileSize, 1, _inputFile) != 1)
    {
        Log->WriteLogNoTime("FAILED: Unable to read file.\n");
        Log->WriteLog("\n");
        return false;
    }

    if (NullPoniterToData())
    {
        Log->WriteLogNoTime("FATAL: DATA_READ_ERROR: null Pointer to file data. Report this to fix it.\n");
        Log->WriteLog("\n");
        return false;
    }

    if (!CheckStructure())
        return false;

    Log->WriteLogNoTime("DONE.\n");

    return true;
}

bool module_parser::CheckStructure()
{
    // Para los archivos csv, si fuera solo la palabra "int" serian 4 bytes al menos
    // Para los archivos binarios, debe tener al menos 20 bytes de datos al inicio
    if ( ((GetFileType() == csvFile || FileIsASCII()) && _fileSize < 4) || (!FileIsASCII() && _fileSize < 20) )
    {
        Log->WriteLogNoTime("FAILED: File size is too small. Are you sure is a '%s' file?\n", Shared->GetFileExtensionByFileType(_sFile.Type));
        Log->WriteLog("\n");
        return false;
    }

    if (GetFileType() == csvFile || FileIsASCII())
        ; // Pasar a funcion csv_reader y comprobar el archivo
    else
    {
        switch (GetFileTypeByHeader())
        {
            case dbcFile:
            case adbFile:
            {
                _headerSize = 20;
                _totalRecords = HeaderGetUInt();
                _totalFields = HeaderGetUInt();
                _recordSize = HeaderGetUInt();
                _stringSize = HeaderGetUInt();

                unsigned int _dataBytes = _fileSize - _headerSize - _stringSize;
                unsigned int _stringBytes = _fileSize - _headerSize - _dataBytes;

                if ((_dataBytes != (_totalRecords * _recordSize)) || !_stringSize || (_stringBytes != _stringSize))
                {
                    Log->WriteLogNoTime("FAILED: Structure is damaged.\n");
                    Log->WriteLog("\n");
                    return false;
                }

                if (!_totalRecords || !_totalFields || !_recordSize)
                {
                    Log->WriteLogNoTime("FAILED: No records found.\n");
                    Log->WriteLog("\n");
                    return false;
                }

                _dataBytes = _fileSize - _headerSize - _stringSize;
                _dataTable = new unsigned char [_dataBytes];
                _dataTable = _fileData + _headerOffset;

                _stringBytes = _fileSize - _headerSize - _dataBytes;
                _stringTable = new unsigned char[_stringBytes];
                _stringTable = _fileData + _headerOffset + _dataBytes;
                break;
            }
            case db2File:
                /// 32 bytes del header o 48 bytes si el build > 12880
                //char header[4];             // WDB2 db2
                //unsigned int totalRecords;
                //unsigned int totalFields;
                //unsigned int recordSize;
                //unsigned int stringSize;
                //unsigned int tableHash;
                //unsigned int build;
                //unsigned int unk1;

                /// > 12880
                /// int diff = maxIndexDB2 - unk2DB2 + 1;
                /// fseek(input, diff * 4 + diff * 2, SEEK_CUR); // diff * 4: an index for rows, diff * 2: a memory allocation bank
                //unsigned int unk2;
                //unsigned int maxIndex;
                //unsigned int locales;
                //unsigned int unk3;
                break;
            case wdbitemcacheFile:
            case wdbcreaturecacheFile:
            case wdbgameobjectcacheFile:
            case wdbitemnamecacheFile:
            case wdbitemtextcacheFile:
            case wdbnpccacheFile:
            case wdbpagetextcacheFile:
            case wdbquestcacheFile:
                /// 24 bytes del header + 8 bytes del primer record y su el tama�o del record
                //char header[4];
                //unsigned int revision;
                //char locale[4];
                //unsigned int maxRecordSize;
                //unsigned int unk1;
                //unsigned int unk2;
                /*
                    Para cada registro:
                    unsigned int entry;
                    unsigned int recordSize;
                    unsigned char *restOfrecord; <- aqui hay que saber el formato para leerlo

                    BDIW itemcache.wdb -> se abre de forma especial por que dependen de unos bytes las veces que lee otros bytes
                    BOMW creaturecache.wdb
                    BOGW gameobjectcache.wdb
                    BDNW itemnamecache.wdb
                    XTIW itemtextcache.wdb
                    CPNW npccache.wdb
                    XTPW pagetextcache.wdb
                    TSQW questcache.wdb
                */
                break;
            default:
                Log->WriteLogNoTime("FAILED: Unknown file.\n");
                Log->WriteLog("\n");
                return false;
        }
    }

    return true;
}

void module_parser::ParseFile()
{
    if (GetFileType() == csvFile || FileIsASCII())
        ParseCSVFile();
    else
        ParseBinaryFile();
}

bool module_parser::CreateCSVFile()
{
    string outputFileNameCSV = GetFileName();
    outputFileNameCSV.append(".csv");
    Log->WriteLog("Creating CSV file '%s'... ", outputFileNameCSV.c_str());

    FILE *output = fopen(outputFileNameCSV.c_str(), "w");
    if (!output)
    {
        Log->WriteLogNoTime("FAILED: Unable to create file.");
        Log->WriteLog("\n");
        return false;
    }

    for (unsigned int currentField = 0; currentField < _totalFields; currentField++)
    {
        switch (_sFile.FormatedFieldTypes[currentField])
        {
            case type_FLOAT:  fprintf(output, "float"); break;
            case type_BOOL:   fprintf(output, "bool"); break;
            case type_STRING: fprintf(output, "string"); break;
            case type_INT:    fprintf(output, "int"); break;
            case type_UINT:
            default:          fprintf(output, "uint"); break;
        }

        if (currentField + 1 < _totalFields)
            fprintf(output, ",");
    }
    fprintf(output, "\n");

    for (unsigned int currentRecord = 0; currentRecord < _totalRecords; currentRecord++)
    {
        for (unsigned int currentField = 0; currentField < _totalFields; currentField++)
        {
            if (_stringSize > 1 && _sFile.FormatedFieldTypes[currentField] == type_STRING)
            {
                unsigned int value = GetRecord(currentRecord).GetUInt(currentField);
                if (value)
                {
                    string outText = "\"";
                    for (unsigned int x = value; x < _stringSize; x++)
                    {
                        if (!_stringTable[x])
                            break;

                        if (_stringTable[x] == '"')
                            outText.append("\"");

                        if (_stringTable[x] == '\r')
                        {
                            outText.append("\\r");
                            continue;
                        }

                        if (_stringTable[x] == '\n')
                        {
                            outText.append("\\n");
                            continue;
                        }

                        outText.append(Shared->ToStr(_stringTable[x]));
                    }
                    outText.append("\"");
                    fprintf(output, "%s", outText.c_str());
                }
            }
            else if (_sFile.FormatedFieldTypes[currentField] == type_FLOAT)
                fprintf(output, "%f", GetRecord(currentRecord).GetFloat(currentField));
            else if (_sFile.FormatedFieldTypes[currentField] == type_BOOL)
                fprintf(output, "%u", GetRecord(currentRecord).GetBool(currentField));
            else if (_sFile.FormatedFieldTypes[currentField] == type_INT)
                fprintf(output, "%i", GetRecord(currentRecord).GetInt(currentField));
            else if (_sFile.FormatedFieldTypes[currentField] == type_UINT)
                fprintf(output, "%u", GetRecord(currentRecord).GetUInt(currentField));

            if (currentField + 1 < _totalFields)
                fprintf(output, ",");
        }

        if (currentRecord + 1 < _totalRecords)
            fprintf(output, "\n");
    }

    fclose(output);

    Log->WriteLogNoTime("DONE.\n");

    return true;
}

bool module_parser::CreateDBCFile()
{
    string outputFileNameDBC = GetFileName();
    outputFileNameDBC.append(".dbc");
    Log->WriteLog("Creating DBC file '%s'... ", outputFileNameDBC.c_str());

    for (unsigned int currentRecord = 0; currentRecord < _totalRecords; currentRecord++)
    {
        for (unsigned int currentField = 0; currentField < _totalFields; currentField++)
        {
            if (_stringSize > 1 && _sFile.FormatedFieldTypes[currentField] == type_STRING)
            {
                unsigned int value = GetRecord(currentRecord).GetUInt(currentField);
                if (value)
                {
                    string outText = "";
                    for (unsigned int x = value; x < _stringSize; x++)
                    {
                        if (!_stringTable[x])
                            break;

                        outText.append(Shared->ToStr(_stringTable[x]));
                    }

                    SetUniqueStringTexts(outText);
                }
            }
        }
    }

    if (_stringTexts.size() != _stringSize)
    {
        Log->WriteLogNoTime("FAILED: Unable to create file.");
        Log->WriteLog("\n");
        return false;
    }

    FILE *output;
    fopen_s(&output, outputFileNameDBC.c_str(), "wb");
    if (!output)
    {
        printf("ERROR: '%s': Can't create DBC file.\n", outputFileNameDBC.c_str());
        return false;
    }

    fwrite("WDBC", 4, 1, output);
    fwrite(&_totalRecords, sizeof(_totalRecords), 1, output);
    fwrite(&_totalFields, sizeof(_totalFields), 1, output);
    fwrite(&_recordSize, sizeof(_recordSize), 1, output);
    fwrite(&_stringSize, sizeof(_stringSize), 1, output);

    for (unsigned int currentRecord = 0; currentRecord < _totalRecords; currentRecord++)
    {
        for (unsigned int currentField = 0; currentField < _totalFields; currentField++)
        {
            if (_sFile.FormatedFieldTypes[currentField] == type_FLOAT)
            {
                float value = GetRecord(currentRecord).GetFloat(currentField);
                fwrite(&value, 4, 1, output);
            }
            else if (_sFile.FormatedFieldTypes[currentField] == type_BOOL)
            {
                unsigned int value = GetRecord(currentRecord).GetBool(currentField);
                fwrite(&value, 4, 1, output);
            }
            else if (_sFile.FormatedFieldTypes[currentField] == type_BYTE)
            {
                int value = GetRecord(currentRecord).GetByte(currentField);
                fwrite(&value, 1, 1, output);
            }
            else if (_sFile.FormatedFieldTypes[currentField] == type_UBYTE)
            {
                unsigned int value = GetRecord(currentRecord).GetByte(currentField);
                fwrite(&value, 1, 1, output);
            }
            else if (_sFile.FormatedFieldTypes[currentField] == type_INT)
            {
                int value = GetRecord(currentRecord).GetInt(currentField);
                fwrite(&value, 4, 1, output);
            }
            else if (_sFile.FormatedFieldTypes[currentField] == type_UINT)
            {
                unsigned int value = GetRecord(currentRecord).GetUInt(currentField);
                fwrite(&value, 4, 1, output);
            }
            else
            {
                unsigned int value = GetRecord(currentRecord).GetUInt(currentField);
                fwrite(&value, 4, 1, output);
            }
        }
    }

    fwrite(_stringTexts.c_str(), _stringTexts.size(), 1, output);

    fclose(output);

    Log->WriteLogNoTime("DONE.\n");

    return true;
}

bool module_parser::ParseBinaryFile()
{
    if (IsPreFormatted())
    {
        Log->WriteLog("Parsing formated file...");

        if (_totalFields != _sFile.FormatedTotalFields || _recordSize != _sFile.FormatedRecordSize)
        {
            Log->WriteLogNoTime("FAILED: Formated structure mismatch.\n");
            Log->WriteLog("\n");
            return false;
        }

        Log->WriteLogNoTime("DONE.\n");
    }
    else
    {
        Log->WriteLog("Predicting field types...");

        if (_recordSize / 4 != _totalFields)
        {
            Log->WriteLogNoTime("FAILED: Not supported byte packed format.\n");
            Log->WriteLog("\n");
            return false;
        }

        if (PredictFieldTypes())
        {
            Log->WriteLogNoTime("DONE.\n");

            if (_countFloatFields)
                Log->WriteLog("Total float Fields Predicted: '%u'\n", _countFloatFields);

            if (_countStringFields)
                Log->WriteLog("Total string Fields Predicted: '%u'\n", _countStringFields);

            if (_countBoolFields)
                Log->WriteLog("Total bool Fields Predicted: '%u'\n", _countBoolFields);

            if (_countIntFields)
                Log->WriteLog("Total int Fields Predicted: '%u'\n", _countIntFields);

            if (_countUIntFields)
                Log->WriteLog("Total unsigned int Fields Predicted: '%u'\n", _countUIntFields);

            CreateCSVFile();
            // CreateDBCFile();
        }
    }

    Log->WriteLogEmptyLine();

    return true;
}

bool module_parser::ParseCSVFile()
{
    Log->WriteLog("Not implemented yet.\n");
    Log->WriteLogEmptyLine();

    return true;
}

bool module_parser::PredictFieldTypes()
{
    // Establecemos field type NONE y extablecemos en donde empieza cada field para todos los fields
    SetFieldTypesToNONE();
    SetFieldsOffset();

    // Obtenemos los tipos de Fields
    // 01 - Float System
    for (unsigned int currentField = 0; currentField < _totalFields; currentField++)
    {
        for (unsigned int currentRecord = 0; currentRecord < _totalRecords; currentRecord++)
        {
            float floatValue = GetRecord(currentRecord).GetFloat(currentField);
            if (floatValue)
            {
                string floatStringValue = Shared->ToStr(floatValue);
                int isFloat1 = floatStringValue.find("e");
                int isFloat2 = floatStringValue.find("#");
                int isFloat3 = floatStringValue.find("-nan");
                if (isFloat1 != -1 || isFloat2 != -1 || isFloat3 != -1)
                {
                    _sFile.FormatedFieldTypes[currentField] = type_NONE;
                    break;
                }

                _sFile.FormatedFieldTypes[currentField] = type_FLOAT;
            }
        }
    }

    // 02 - Bool System
    for (unsigned int currentField = 0; currentField < _totalFields; currentField++)
    {
        if (_sFile.FormatedFieldTypes[currentField] == type_FLOAT)
            continue;

        for (unsigned int currentRecord = 0; currentRecord < _totalRecords; currentRecord++)
        {
            int intValue = GetRecord(currentRecord).GetInt(currentField);

            if (intValue < 0 || intValue > 1)
            {
                _sFile.FormatedFieldTypes[currentField] = type_NONE;
                break;
            }

            _sFile.FormatedFieldTypes[currentField] = type_BOOL;
        }
    }

    // 03 - String System
    if (_stringSize > 1)
    {
        for (unsigned int currentField = 0; currentField < _totalFields; currentField++)
        {
            if (_sFile.FormatedFieldTypes[currentField] == type_FLOAT || _sFile.FormatedFieldTypes[currentField] == type_BOOL)
                continue;

            for (unsigned int currentRecord = 0; currentRecord < _totalRecords; currentRecord++)
            {
                int intValue = GetRecord(currentRecord).GetUInt(currentField);
                if (intValue < 0 || intValue >= int(_stringSize) || (intValue > 0 && _stringTable[intValue - 1]))
                {
                    _sFile.FormatedFieldTypes[currentField] = type_INT;
                    break;
                }

                _sFile.FormatedFieldTypes[currentField] = type_STRING;
            }
        }
    }

    // 04 - Unsigned/Signed Int System
    for (unsigned int currentField = 0; currentField < _totalFields; currentField++)
    {
        if (_sFile.FormatedFieldTypes[currentField] == type_FLOAT || _sFile.FormatedFieldTypes[currentField] == type_BOOL || _sFile.FormatedFieldTypes[currentField] == type_STRING)
            continue;

        for (unsigned int currentRecord = 0; currentRecord < _totalRecords; currentRecord++)
        {
            int intValue = GetRecord(currentRecord).GetInt(currentField);
            if (intValue < 0)
            {
                _sFile.FormatedFieldTypes[currentField] = type_INT;
                break;
            }

            _sFile.FormatedFieldTypes[currentField] = type_UINT;
        }
    }

    for (unsigned int currentField = 0; currentField < _totalFields; currentField++)
    {
        if (_sFile.FormatedFieldTypes[currentField] == type_FLOAT)
            _countFloatFields++;
        if (_sFile.FormatedFieldTypes[currentField] == type_STRING)
            _countStringFields++;
        if (_sFile.FormatedFieldTypes[currentField] == type_BOOL)
            _countBoolFields++;
        if (_sFile.FormatedFieldTypes[currentField] == type_INT)
            _countIntFields++;
        if (_sFile.FormatedFieldTypes[currentField] == type_UINT)
            _countUIntFields++;
    }

    if ((_countFloatFields + _countStringFields + _countBoolFields + _countIntFields + _countUIntFields) != _totalFields)
    {
        Log->WriteLogNoTime("FAILED: One or more fields are not predicted. Conctact Developer to fix it.\n");
        return false;
    }

    return true;
}