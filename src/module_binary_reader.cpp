#include "module_binary_reader.h"

bool BinaryReader::Load()
{
    _inputFile = fopen(_fileName, "rb");
    if (!_inputFile)
    {
        printf("ERROR: Can't open file '%s'.\n", _fileName);
        return false;
    }

    printf("Reading file '%s'...", _fileName);

    fseek(_inputFile, 0, SEEK_END);
    _fileSize = ftell(_inputFile);

    if (!_fileSize)
    {
        printf("FAILED: Empty File.\n");
        return false;
    }

    rewind(_inputFile);

    structDBCHeader DBCHeader;
    if (_fileSize < 20 || fread(&DBCHeader, _headerSize, 1, _inputFile) != 1)
    {
        printf("FAILED: File size is too small. Are you sure is a DBC file?\n");
        return false;
    }

    if (DBCHeader.header[0] != 'W' && DBCHeader.header[1] != 'D' && DBCHeader.header[2] != 'B' && DBCHeader.header[3] != 'C')
    {
        printf("FAILED: Not a DBC file.\n");
        return false;
    }

    printf("DONE.\n");

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
            printf("FAILED: Formated structure mismatch.\n");
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

        return false;
    }
}

bool BinaryReader::CheckStructure()
{
    printf("Checking structure...");

    _dataBytes = _fileSize - _headerSize - _stringSize;
    _stringBytes = _fileSize - _headerSize - _dataBytes;
    if ((_dataBytes != (_totalRecords * _recordSize)) || !_stringSize || (_stringBytes != _stringSize))
    {
        printf("FAILED: Structure is damaged.\n");
        return false;
    }

    if (!_totalRecords || !_totalFields || !_recordSize)
    {
        printf("FAILED: No records/fields found.\n");
        return false;
    }

    _dataTable = new unsigned char[_dataBytes];
    if (fread(_dataTable, _dataBytes, 1, _inputFile) != 1)
    {
        printf("FAILED: Unable to read records data.\n");
        return false;
    }

    _stringTable = new unsigned char[_stringBytes];
    if (fread(_stringTable, _stringBytes, 1, _inputFile) != 1)
    {
        printf("FAILED: Unable to read strings data.\n");
        return false;
    }

    printf("DONE.\n");

    return true;
}

bool BinaryReader::PredictFieldTypes()
{
    printf("Predicting field types...");

    if (_recordSize / 4 != _totalFields)
    {
        printf("FAILED: Not supported byte packed format.\n");
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
                string floatStringValue = ToStr(floatValue);
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

    // 03 - Unsigned/Signed Int System
    for (unsigned int currentField = 0; currentField < _totalFields; currentField++)
    {
        if (_fieldTypes[currentField] == type_FLOAT || _fieldTypes[currentField] == type_BOOL)
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

    // 04 - String System
    if (_stringSize > 1)
    {
        for (unsigned int currentField = 0; currentField < _totalFields; currentField++)
        {
            if (_fieldTypes[currentField] == type_FLOAT || _fieldTypes[currentField] == type_BOOL || _fieldTypes[currentField] == type_INT)
                continue;

            for (unsigned int currentRecord = 0; currentRecord < _totalRecords; currentRecord++)
            {
                int intValue = GetRecord(currentRecord).GetInt(currentField);
                if (intValue >= int(_stringSize) || (intValue > 0 && _stringTable[intValue - 1]))
                {
                    _fieldTypes[currentField] = type_UINT;
                    break;
                }

                _fieldTypes[currentField] = type_STRING;
            }
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
        printf("FAILED: One or more fields are not predicted. Conctact Developer to fix it.\n");
        return false;
    }

    printf("DONE.\n");

    if (countFloat)
        printf("Total float Fields Predicted: '%u'\n", countFloat);

    if (countString)
        printf("Total string Fields Predicted: '%u'\n", countString);

    if (countBool)
        printf("Total bool Fields Predicted: '%u'\n", countBool);

    if (countInt)
        printf("Total int Fields Predicted: '%u'\n", countInt);

    if (countUInt)
        printf("Total unsigned int Fields Predicted: '%u'\n", countUInt);

    return true;
}
