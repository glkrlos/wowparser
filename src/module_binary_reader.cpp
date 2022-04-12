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
        fclose(_inputFile);
        return false;
    }

    if (DBCHeader.header[0] != 'W' && DBCHeader.header[1] != 'D' && DBCHeader.header[2] != 'B' && DBCHeader.header[3] != 'C')
    {
        printf("FAILED: Not a DBC file.\n");
        fclose(_inputFile);
        return false;
    }

    printf("DONE.\n");

    _totalRecords = DBCHeader.totalRecords;
    _totalFields = DBCHeader.totalFields;
    _recordSize = DBCHeader.recordSize;
    _stringSize = DBCHeader.stringSize;

    if (!CheckStructure())
        return false;

    return false;
}

bool BinaryReader::CheckStructure()
{
    /*
    printf("Checking structure...");
    if (isFormated())
    {
        if (TotalFields != FormatedTotalFields || RecordSize != FormatedRecordSize)
        {
            printf("FAILED: Formated structure mismatch.\n");
            fclose(input);
            return false;
        }
    }

    long dataBytes = FileSize - HeaderSize - StringSize;
    long stringBytes = FileSize - HeaderSize - dataBytes;
    if ((dataBytes != (TotalRecords * RecordSize)) || !StringSize || (stringBytes != StringSize))
    {
        printf("FAILED: Structure is damaged.\n");
        fclose(input);
        return false;
    }

    if (!TotalRecords || !TotalFields || !RecordSize)
    {
        printf("FAILED: No records/fields found.\n");
        fclose(input);
        return false;
    }

    DataTable = new unsigned char[dataBytes];
    if (fread(DataTable, dataBytes, 1, input) != 1)
    {
        printf("FAILED: Unable to read records data.\n");
        fclose(input);
        return false;
    }

    StringTable = new unsigned char[StringSize];
    if (fread(StringTable, StringSize, 1, input) != 1)
    {
        printf("FAILED: Unable to read strings data.\n");
        fclose(input);
        return false;
    }
    fclose(input);

    printf("DONE.\n");

    if (isFormated())
    {
        SetFieldsOffset();
        return true;
    }

    if (!PredictFieldTypes())
        return false;
*/
    return true;
}

bool BinaryReader::PredictFieldTypes()
{
    /*
    printf("Predicting field types...");

    if (RecordSize / 4 != TotalFields)
    {
        printf("FAILED: Not supported byte packed format.\n");
        return false;
    }

    // Establecemos field type NONE y extablecemos en donde empieza cada field para todos los fields
    SetFieldTypesToNONE();
    SetFieldsOffset();

    // Obtenemos los tipos de Fields
    // Float System
    for (unsigned int currentField = 0; currentField < TotalFields; currentField++)
    {
        for (unsigned int currentRecord = 0; currentRecord < TotalRecords; currentRecord++)
        {
            float floatValue = GetRecord(currentRecord).GetFloat(currentField);
            if (floatValue)
            {
                string floatStringValue = ToStr(floatValue);
                int isFloat1 = floatStringValue.find("e");
                int isFloat2 = floatStringValue.find("#");
                if (isFloat1 != -1 || isFloat2 != -1)
                {
                    FieldTypes[currentField] = type_NONE;
                    break;
                }

                FieldTypes[currentField] = type_FLOAT;
            }
        }
    }

    // String System
    if (StringSize > 1)
    {
        for (unsigned int currentField = 0; currentField < TotalFields; currentField++)
        {
            if (FieldTypes[currentField] == type_FLOAT)
                continue;

            for (unsigned int currentRecord = 0; currentRecord < TotalRecords; currentRecord++)
            {
                int intValue = GetRecord(currentRecord).GetInt(currentField);
                if (intValue < 0 || intValue >= int(StringSize) || (intValue > 0 && StringTable[intValue - 1]))
                {
                    FieldTypes[currentField] = type_NONE;
                    break;
                }

                FieldTypes[currentField] = type_STRING;
            }
        }
    }

    // Bool System
    for (unsigned int currentField = 0; currentField < TotalFields; currentField++)
    {
        if (FieldTypes[currentField] == type_FLOAT || FieldTypes[currentField] == type_STRING)
            continue;

        for (unsigned int currentRecord = 0; currentRecord < TotalRecords; currentRecord++)
        {
            int intValue = GetRecord(currentRecord).GetInt(currentField);

            if (intValue < 0 || intValue > 1)
            {
                FieldTypes[currentField] = type_NONE;
                break;
            }

            FieldTypes[currentField] = type_BOOL;
        }
    }

    // Unsigned/Signed Int System
    for (unsigned int currentField = 0; currentField < TotalFields; currentField++)
    {
        if (FieldTypes[currentField] == type_FLOAT || FieldTypes[currentField] == type_STRING || FieldTypes[currentField] == type_BOOL)
            continue;

        for (unsigned int currentRecord = 0; currentRecord < TotalRecords; currentRecord++)
        {
            int intValue = GetRecord(currentRecord).GetInt(currentField);
            if (intValue < 0)
            {
                FieldTypes[currentField] = type_INT;
                break;
            }

            FieldTypes[currentField] = type_UINT;
        }
    }

    unsigned int countFloat = 0;
    unsigned int countString = 0;
    unsigned int countBool = 0;
    unsigned int countInt = 0;
    unsigned int countUInt = 0;
    for (unsigned int currentField = 0; currentField < TotalFields; currentField++)
    {
        if (FieldTypes[currentField] == type_FLOAT)
            countFloat++;
        if (FieldTypes[currentField] == type_STRING)
            countString++;
        if (FieldTypes[currentField] == type_BOOL)
            countBool++;
        if (FieldTypes[currentField] == type_INT)
            countInt++;
        if (FieldTypes[currentField] == type_UINT)
            countUInt++;
    }

    if ((countFloat + countString + countBool + countInt + countUInt) != TotalFields)
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
*/
    return true;
}
