#include "dbc_reader.h"

bool DBCReader::Load()
{
    input = fopen(FileName, "rb");
    if (!input)
    {
        // printf("ERROR: Can't open file '%s'.\n", fileName);
        return false;
    }

    // printf("Reading file '%s'...", fileName);

    fseek(input, 0, SEEK_END);
    FileSize = ftell(input);
    rewind(input);

    structDBCHeader DBCHeader;
    if (FileSize < 20 || fread(&DBCHeader, HeaderSize, 1, input) != 1)
    {
        // printf("FAILED: File size is too small. Are you sure is a DBC file?\n");
        fclose(input);
        return false;
    }

    if (DBCHeader.header[0] == 'W' && DBCHeader.header[1] == 'D' && DBCHeader.header[2] == 'B' && DBCHeader.header[3] == 'C')
    {
        TotalRecords = DBCHeader.totalRecords;
        TotalFields = DBCHeader.totalFields;
        RecordSize = DBCHeader.recordSize;
        StringSize = DBCHeader.stringSize;

        if (!CheckStructure())
            return false;

        return true;
    }

    // printf("FAILED: Not a DBC file.\n");

    return false;
}

bool DBCReader::CheckStructure()
{
    printf("Loading file '%s'...", FileName);
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

    if (!TotalRecords || !TotalFields || !RecordSize )
    {
        printf("FAILED: No records/fields found.\n");
        fclose(input);
        return false;
    }

    Data = new unsigned char[dataBytes];
    if (fread(Data, dataBytes, 1, input) != 1)
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

    printf("DONE");

    if (isFormated())
    {
        SetFieldsOffset();
        printf(" (Formated)\n");
        return true;
    }

    printf(" (Predicted)\n");
    if (!PredictFieldTypes())
        return false;

    return true;
}

bool DBCReader::PredictFieldTypes()
{
    //printf("Predicting field types...");
    if (RecordSize / TotalFields != 4)
    {
        if (RecordSize % 4 != 0)
        {
            printf("\tFAILED: Not supported byte packed format.\n");
            return false;
        }
        TotalFields = RecordSize / 4;
    }

    // Establecemos field type NONE para todos los fields
    SetFieldTypesToNONE();
    SetFieldsOffset();

    // Obtenemos los tipos de Fields
    // Float System
    for (unsigned int currentField = 0; currentField < TotalFields; currentField++)
    {
        for (unsigned int currentRecord = 0; currentRecord < TotalRecords; currentRecord++)
        {
            GetRecord(currentRecord);
            float floatValue = GetFloat(currentField);
            if (floatValue)
            {
                string floatStringValue = ToStr(floatValue);
                int isFloat1 = floatStringValue.find("e");
                int isFloat2 = floatStringValue.find("#");
                if ((isFloat1 == -1) && (isFloat2 == -1))
                    FieldTypes[currentField] = type_FLOAT;
                else
                    FieldTypes[currentField] = type_NONE;
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
                GetRecord(currentRecord);
                int intValue = GetInt(currentField);
                if ((intValue < 0) || (intValue >= int(StringSize)))
                {
                    FieldTypes[currentField] = type_NONE;
                    break;
                }
                else if ((intValue > 0) && (StringTable[intValue - 1]))
                {
                    FieldTypes[currentField] = type_NONE;
                    break;
                }
                else if ((intValue > 0) && !StringTable[intValue - 1])
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
            GetRecord(currentRecord);
            int intValue = GetInt(currentField);

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
            GetRecord(currentRecord);
            int intValue = GetInt(currentField);
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
    //printf("DONE\n");

    if (countFloat)
        printf("Float Fields:\t'%u'\n", countFloat);
    if (countString)
        printf("String Fields:\t'%u'\n", countString);
    if (countBool)
        printf("Bool Fields:\t'%u'\n", countBool);
    if (countInt)
        printf("Int Fields:\t'%u'\n", countInt);
    if (countUInt)
        printf("Uint Fields:\t'%u'\n", countUInt);

    return true;
}