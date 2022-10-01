#include "module_parser.h"
#include "log.h"

bool module_parser::Load()
{
    Log->WriteLogEmptyLine();

    _inputFile = fopen(_fileName, "rb");
    if (!_inputFile)
    {
        Log->WriteLog("ERROR: Can't open file '%s'.\n", _fileName);
        return false;
    }

    Log->WriteLog("Reading file '%s'...", _fileName);

    fseek(_inputFile, 0, SEEK_END);
    _fileSize = ftell(_inputFile);

    if (!_fileSize)
    {
        Log->WriteLogNoTime("FAILED: Empty File.\n");
        return false;
    }

    rewind(_inputFile);

    structDBCHeader DBCHeader;
    if (_fileSize < 20 || fread(&DBCHeader, _headerSize, 1, _inputFile) != 1)
    {
        Log->WriteLogNoTime("FAILED: File size is too small. Are you sure is a DBC file?\n");
        return false;
    }

    if (DBCHeader.header[0] != 'W' && DBCHeader.header[1] != 'D' && DBCHeader.header[2] != 'B' && DBCHeader.header[3] != 'C')
    {
        Log->WriteLogNoTime("FAILED: Not a DBC file.\n");
        return false;
    }

    Log->WriteLogNoTime("DONE.\n");

    _totalRecords = DBCHeader.totalRecords;
    _totalFields = DBCHeader.totalFields;
    _recordSize = DBCHeader.recordSize;
    _stringSize = DBCHeader.stringSize;

    if (!CheckStructure())
        return false;

    if (isFormated())
    {
        if (_totalFields != _formatedTotalFields || _recordSize != _formatedRecordSize)
        {
            Log->WriteLogNoTime("FAILED: Formated structure mismatch.\n");
            return false;
        }

        SetFieldsOffset();

        // funcion para leer y duardar la informacion

        return true;
    }
    else
    {
        if (!PredictFieldTypes())
            return false;

        return true;
    }
}

bool module_parser::CheckStructure()
{
    Log->WriteLog("Checking structure...");

    _dataBytes = _fileSize - _headerSize - _stringSize;
    _stringBytes = _fileSize - _headerSize - _dataBytes;
    if ((_dataBytes != (_totalRecords * _recordSize)) || !_stringSize || (_stringBytes != _stringSize))
    {
        Log->WriteLogNoTime("FAILED: Structure is damaged.\n");
        return false;
    }

    if (!_totalRecords || !_totalFields || !_recordSize)
    {
        Log->WriteLogNoTime("FAILED: No records/fields found.\n");
        return false;
    }

    _dataTable = new unsigned char[_dataBytes];
    if (fread(_dataTable, _dataBytes, 1, _inputFile) != 1)
    {
        Log->WriteLogNoTime("FAILED: Unable to read records data.\n");
        return false;
    }

    _stringTable = new unsigned char[_stringBytes];
    if (fread(_stringTable, _stringBytes, 1, _inputFile) != 1)
    {
        Log->WriteLogNoTime("FAILED: Unable to read strings data.\n");
        return false;
    }

    Log->WriteLogNoTime("DONE.\n");

    return true;
}

bool module_parser::PredictFieldTypes()
{
    Log->WriteLog("Predicting field types...");

    if (_recordSize / 4 != _totalFields)
    {
        Log->WriteLogNoTime("FAILED: Not supported byte packed format.\n");
        return false;
    }

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
                if (isFloat1 != -1 || isFloat2 != -1)
                {
                    _fieldTypes[currentField] = type_NONE;
                    break;
                }

                _fieldTypes[currentField] = type_FLOAT;
            }
        }
    }

    // 02 - Bool System
    for (unsigned int currentField = 0; currentField < _totalFields; currentField++)
    {
        if (_fieldTypes[currentField] == type_FLOAT)
            continue;

        for (unsigned int currentRecord = 0; currentRecord < _totalRecords; currentRecord++)
        {
            int intValue = GetRecord(currentRecord).GetInt(currentField);

            if (intValue < 0 || intValue > 1)
            {
                _fieldTypes[currentField] = type_NONE;
                break;
            }

            _fieldTypes[currentField] = type_BOOL;
        }
    }

    // 03 - String System
    if (_stringSize > 1)
    {
        for (unsigned int currentField = 0; currentField < _totalFields; currentField++)
        {
            if (_fieldTypes[currentField] == type_FLOAT || _fieldTypes[currentField] == type_BOOL)
                continue;

            for (unsigned int currentRecord = 0; currentRecord < _totalRecords; currentRecord++)
            {
                int intValue = GetRecord(currentRecord).GetUInt(currentField);
                if (intValue < 0 || intValue >= int(_stringSize) || (intValue > 0 && _stringTable[intValue - 1]))
                {
                    _fieldTypes[currentField] = type_INT;
                    break;
                }

                _fieldTypes[currentField] = type_STRING;
            }
        }
    }

    // 04 - Unsigned/Signed Int System
    for (unsigned int currentField = 0; currentField < _totalFields; currentField++)
    {
        if (_fieldTypes[currentField] == type_FLOAT || _fieldTypes[currentField] == type_BOOL || _fieldTypes[currentField] == type_STRING)
            continue;

        for (unsigned int currentRecord = 0; currentRecord < _totalRecords; currentRecord++)
        {
            int intValue = GetRecord(currentRecord).GetInt(currentField);
            if (intValue < 0)
            {
                _fieldTypes[currentField] = type_INT;
                break;
            }

            _fieldTypes[currentField] = type_UINT;
        }
    }

    unsigned int countFloat = 0;
    unsigned int countString = 0;
    unsigned int countBool = 0;
    unsigned int countInt = 0;
    unsigned int countUInt = 0;
    for (unsigned int currentField = 0; currentField < _totalFields; currentField++)
    {
        if (_fieldTypes[currentField] == type_FLOAT)
            countFloat++;
        if (_fieldTypes[currentField] == type_STRING)
            countString++;
        if (_fieldTypes[currentField] == type_BOOL)
            countBool++;
        if (_fieldTypes[currentField] == type_INT)
            countInt++;
        if (_fieldTypes[currentField] == type_UINT)
            countUInt++;
    }

    if ((countFloat + countString + countBool + countInt + countUInt) != _totalFields)
    {
        Log->WriteLogNoTime("FAILED: One or more fields are not predicted. Conctact Developer to fix it.\n");
        return false;
    }

    Log->WriteLogNoTime("DONE.\n");

    if (countFloat)
        Log->WriteLog("Total float Fields Predicted: '%u'\n", countFloat);

    if (countString)
        Log->WriteLog("Total string Fields Predicted: '%u'\n", countString);

    if (countBool)
        Log->WriteLog("Total bool Fields Predicted: '%u'\n", countBool);

    if (countInt)
        Log->WriteLog("Total int Fields Predicted: '%u'\n", countInt);

    if (countUInt)
        Log->WriteLog("Total unsigned int Fields Predicted: '%u'\n", countUInt);

    string outputFileName = _fileName;
    outputFileName.append(".csv");
    FILE *output = fopen(outputFileName.c_str(), "w");
    if (!output)
    {
        Log->WriteLog("ERROR: File cannot be created '%s'.\n", outputFileName.c_str());
        return false;
    }

    for (unsigned int currentField = 0; currentField < _totalFields; currentField++)
    {
        switch (_fieldTypes[currentField])
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
            if (_stringSize > 1 && _fieldTypes[currentField] == type_STRING)
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
            else if (_fieldTypes[currentField] == type_FLOAT)
                fprintf(output, "%f", GetRecord(currentRecord).GetFloat(currentField));
            else if (_fieldTypes[currentField] == type_BOOL)
                fprintf(output, "%u", GetRecord(currentRecord).GetBool(currentField));
            else if (_fieldTypes[currentField] == type_INT)
                fprintf(output, "%i", GetRecord(currentRecord).GetInt(currentField));
            else if (_fieldTypes[currentField] == type_UINT)
                fprintf(output, "%i", GetRecord(currentRecord).GetInt(currentField));

            if (currentField + 1 < _totalFields)
                fprintf(output, ",");
        }
        fprintf(output, "\n");
    }

    fclose(output);

    Log->WriteLog("CSV file created: '%s'.\n", outputFileName.c_str());

    return true;
}
