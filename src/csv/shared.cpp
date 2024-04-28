#include "shared.h"

namespace Csv {
    bool ExtractFields(string originalText, map<unsigned int, string> &mapFields)
    {
        unsigned int fieldID = 0;
        if (originalText.empty())
        {
            mapFields.insert(pair<unsigned int, string>(fieldID++, ""));
            return true;
        }

        string _fieldData;
        for (unsigned int x = 0; x < originalText.size(); x++)
        {
            bool isFirstChar = (x == 0);
            bool isLastChar = (x + 1) >= originalText.size();

            if (originalText[x] == ',')
            {
                if (isFirstChar)
                {
                    mapFields.insert(pair<unsigned int, string>(fieldID++, ""));
                    continue;
                }

                mapFields.insert(pair<unsigned int, string>(fieldID++, _fieldData));
                _fieldData.clear();

                if (isLastChar)
                {
                    mapFields.insert(pair<unsigned int, string>(fieldID++, ""));
                    break;
                }

                continue;
            }
            else
                _fieldData += originalText[x];

            isLastChar = (x + 1) >= originalText.size();

            if (isLastChar)
            {
                mapFields.insert(pair<unsigned int, string>(fieldID++, _fieldData));
                _fieldData.clear();
                break;
            }

        }

        return true;
    }

    const char* GetFieldTypeName(enumFieldTypes fieldType)
    {
        switch (fieldType)
        {
            case type_STRING:   return "string";
            case type_FLOAT:    return "float";
            case type_BYTE:     return "byte";
            case type_UBYTE:    return "unsigned byte";
            case type_INT:      return "int";
            case type_UINT:     return "unsigned int";
            case type_BOOL:
            default:            return "bool";
        }
    }

    bool CheckFieldValue(unsigned int fieldID, enumFieldTypes fieldType, string fieldValue, unsigned int recordID)
    {
        bool isFloat = fieldType == type_FLOAT;
        bool isBool = fieldType == type_BOOL;
        bool isUnsigned = (fieldType == type_UINT || fieldType == type_UBYTE);

        if (fieldValue.empty())
        {
            Log->WriteLogNoTime("FAILED: Field '%u' Type '%s' Line '%u' Can't be empty. If you want to leave it empty, put value of '0' instead.\n", fieldID + 1, GetFieldTypeName(fieldType), recordID + 1);
            return false;
        }

        int DotFirst = (int)fieldValue.find('.');
        int DotSecond = (int)fieldValue.rfind('.');

        if (!isFloat && DotFirst != -1)
        {
            Log->WriteLogNoTime("FAILED: Field '%u' Type '%s' Line '%u' Can't contain dot symbol '.'\n", fieldID + 1, GetFieldTypeName(fieldType), recordID + 1);
            return false;
        }

        if (isFloat)
        {
            if (DotFirst != -1 && DotSecond != -1 && DotFirst != DotSecond)
            {
                Log->WriteLogNoTime("FAILED: Field '%u' Type '%s' Line '%u' Can't contains more than one dot symbol '.'\n", fieldID + 1, GetFieldTypeName(fieldType), recordID + 1);
                return false;
            }

            if (DotFirst == 0 && (fieldValue.size() - 1) == 0)
            {
                Log->WriteLogNoTime("FAILED: Field '%u' Type '%s' Line '%u' Dot symbol '.' can't be the only character in field value.\n", fieldID + 1, GetFieldTypeName(fieldType), recordID + 1);
                return false;
            }

            if (DotFirst == 0)
            {
                Log->WriteLogNoTime("FAILED: Field '%u' Type '%s' Line '%u' Dot symbol '.' can't be at the start of field value.\n", fieldID + 1, GetFieldTypeName(fieldType), recordID + 1);
                return false;
            }

            if (DotFirst == ((int)fieldValue.size() - 1))
            {
                Log->WriteLogNoTime("FAILED: Field '%u' Type '%s' Line '%u' Dot symbol '.' can't be the last character in field value.\n", fieldID + 1, GetFieldTypeName(fieldType), recordID + 1);
                return false;
            }
        }

        int NegativeFirst = (int)fieldValue.find('-');
        int NegativeSecond = (int)fieldValue.rfind('-');

        if ((isBool || isUnsigned) && NegativeFirst != -1)
        {
            Log->WriteLogNoTime("FAILED: Field '%u' Type '%s' Line '%u' Can't contain negative symbol '-' in field value.\n", fieldID + 1, GetFieldTypeName(fieldType), recordID + 1);
        }

        if (!isBool && !isUnsigned)
        {
            if (NegativeFirst != -1 && NegativeSecond != -1 && NegativeFirst != NegativeSecond)
            {
                Log->WriteLogNoTime("FAILED: Field '%u' Type '%s' Line '%u' Can't contains more than one negative symbol '-' in field value.\n", fieldID + 1, GetFieldTypeName(fieldType), recordID + 1);
                return false;
            }

            if (NegativeFirst > 0)
            {
                Log->WriteLogNoTime("FAILED: Field '%u' Type '%s' Line '%u' Negative symbol '-' only can be at the start of field value.\n", fieldID + 1, GetFieldTypeName(fieldType), recordID + 1);
                return false;
            }

            if (NegativeFirst == 0 && (fieldValue.size() - 1) == 0)
            {
                Log->WriteLogNoTime("FAILED: Field '%u' Type '%s' Line '%u' Negative symbol '-' can't be the only character in field value.\n", fieldID + 1, GetFieldTypeName(fieldType), recordID + 1);
                return false;
            }

            if (NegativeFirst == ((int)fieldValue.size() - 1))
            {
                Log->WriteLogNoTime("FAILED: Field '%u' Type '%s' Line '%u' Negative symbol '-' can't be the last character in field value.\n", fieldID + 1, GetFieldTypeName(fieldType), recordID + 1);
                return false;
            }
        }

        for (char currentChar : fieldValue)
        {
            if (isFloat && currentChar == '.')
                continue;

            if ((currentChar == '-') || (currentChar == '3') || (currentChar == '7') ||
                (currentChar == '0') || (currentChar == '4') || (currentChar == '8') ||
                (currentChar == '1') || (currentChar == '5') || (currentChar == '9') ||
                (currentChar == '2') || (currentChar == '6'))
                continue;

            Log->WriteLogNoTime("FAILED: Field '%u' Type '%s' Line '%u' Contains a non numeric value.\n", fieldID + 1, GetFieldTypeName(fieldType), recordID + 1);

            return false;
        }

        int testValue = atoi(fieldValue.c_str());

        if (fieldType == type_BOOL && (testValue < 0 || testValue > 1))
        {
            Log->WriteLogNoTime("FAILED: Field '%u' Type '%s' Line '%u' Value can be only '0' or '1'.\n", fieldID + 1, GetFieldTypeName(fieldType), recordID + 1);
            return false;
        }

        if (fieldType == type_BYTE && (testValue < -127 || testValue > 127))
        {
            Log->WriteLogNoTime("FAILED: Field '%u' Type '%s' Line '%u' Value can be only between '-127' and '127'.\n", fieldID + 1, GetFieldTypeName(fieldType), recordID + 1);
            return false;
        }

        if (fieldType == type_UBYTE && (testValue < 0 || testValue > 255))
        {
            Log->WriteLogNoTime("FAILED: Field '%u' Type '%s' Line '%u' Value can be only between '0' and '255'.\n", fieldID + 1, GetFieldTypeName(fieldType), recordID + 1);
            return false;
        }

        return true;
    }

} // Csv
