#include "dbc_reader.h"

bool DBCReader::Load()
{
    sLog->WriteLog("\n");
    input = fopen(FileName, "rb");
    if (!input)
    {
        sLog->WriteLog("DBCReader::Load(): Can't open file '%s'.\n", FileName);
        return false;
    }

    sLog->WriteLog("%s: Reading file '%s'...", __PRETTY_FUNCTION__, FileName); // __FUNCTION__ __func__ 
    // #define LogFile(fmt, ...) \
    // LogFileFunc(__FILE__, __LINE__, fmt, __VA_ARGS__)

    fseek(input, 0, SEEK_END);
    FileSize = ftell(input);

    if (!FileSize)
    {
        sLog->WriteLogNoTime("FAILED: Empty File.\n");
        return false;
    }

    rewind(input);

    structDBCHeader DBCHeader;
    if (FileSize < 20 || fread(&DBCHeader, HeaderSize, 1, input) != 1)
    {
        sLog->WriteLogNoTime("FAILED: File size is too small. Are you sure is a DBC file?\n");
        fclose(input);
        return false;
    }

    if (DBCHeader.header[0] != 'W' && DBCHeader.header[1] != 'D' && DBCHeader.header[2] != 'B' && DBCHeader.header[3] != 'C')
    {
        sLog->WriteLogNoTime("FAILED: Not a DBC file.\n");
        fclose(input);
        return false;
    }

    sLog->WriteLogNoTime("DONE.\n");

    TotalRecords = DBCHeader.totalRecords;
    TotalFields = DBCHeader.totalFields;
    RecordSize = DBCHeader.recordSize;
    StringSize = DBCHeader.stringSize;

    if (!CheckStructure())
        return false;

    return false;
}

bool DBCReader::CheckStructure()
{
    sLog->WriteLog("DBCReader::CheckStructure(): Checking structure...");
    if (isFormated())
    {
        if (TotalFields != FormatedTotalFields || RecordSize != FormatedRecordSize)
        {
            sLog->WriteLogNoTime("FAILED: Formated structure mismatch.\n");
            fclose(input);
            return false;
        }
    }

    long dataBytes = FileSize - HeaderSize - StringSize;
    long stringBytes = FileSize - HeaderSize - dataBytes;
    if ((dataBytes != (TotalRecords * RecordSize)) || !StringSize || (stringBytes != StringSize))
    {
        sLog->WriteLogNoTime("FAILED: Structure is damaged.\n");
        fclose(input);
        return false;
    }

    if (!TotalRecords || !TotalFields || !RecordSize )
    {
        sLog->WriteLogNoTime("FAILED: No records/fields found.\n");
        fclose(input);
        return false;
    }

    Data = new unsigned char[dataBytes];
    if (fread(Data, dataBytes, 1, input) != 1)
    {
        sLog->WriteLogNoTime("FAILED: Unable to read records data.\n");
        fclose(input);
        return false;
    }

    StringTable = new unsigned char[StringSize];
    if (fread(StringTable, StringSize, 1, input) != 1)
    {
        sLog->WriteLogNoTime("FAILED: Unable to read strings data.\n");
        fclose(input);
        return false;
    }
    fclose(input);

    sLog->WriteLogNoTime("DONE.\n");

    if (isFormated())
    {
        SetFieldsOffset();
        return true;
    }

    if (!PredictFieldTypes())
        return false;

    return true;
}

bool DBCReader::PredictFieldTypes()
{
    sLog->WriteLog("DBCReader::PredictFieldTypes(): Predicting field types...");

    if (RecordSize / 4 != TotalFields)
    {
        sLog->WriteLogNoTime("FAILED: Not supported byte packed format.\n");
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
        sLog->WriteLogNoTime("FAILED: One or more fields are not predicted. Conctact Developer to fix it.\n");
        return false;
    }

    sLog->WriteLogNoTime("DONE.\n");

    if (countFloat)
        sLog->WriteLog("DBCReader::PredictFieldTypes(): Total float Fields Predicted: '%u'\n", countFloat);

    if (countString)
        sLog->WriteLog("DBCReader::PredictFieldTypes(): Total string Fields Predicted: '%u'\n", countString);

    if (countBool)
        sLog->WriteLog("DBCReader::PredictFieldTypes(): Total bool Fields Predicted: '%u'\n", countBool);

    if (countInt)
        sLog->WriteLog("DBCReader::PredictFieldTypes(): Total int Fields Predicted: '%u'\n", countInt);

    if (countUInt)
        sLog->WriteLog("DBCReader::PredictFieldTypes(): Total unsigned int Fields Predicted: '%u'\n", countUInt);

    return true;
}
