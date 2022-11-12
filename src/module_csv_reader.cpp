#include "module_csv_reader.h"

CSV_Reader::CSV_Reader(const char *fileName)
{
    _fileName = fileName;

    _recordSize = 0;
    _totalRecords = 0;
    _totalFields = 0;

    _fileData.clear();
    _fieldTypes.clear();
}

CSV_Reader::~CSV_Reader()
{
    _fileName = NULL;

    _recordSize = 0;
    _totalRecords = 0;
    _totalFields = 0;

    _fileData.clear();
    _fieldTypes.clear();
}

bool CSV_Reader::LoadCSVFile()
{
    // abrimos el archivo de texto como solo lectura
    ifstream input(_fileName, ifstream::in);
    if (!input.is_open())
    {
        printf("ERROR: Can't open file '%s' (File exists?).\n", _fileName);
        return false;
    }

    // si el archivo existe entonces mostramos el mensaje
    printf("Loading file '%s'...\n", _fileName);

    // la primera linea no puede estar vacia nunca
    string FirstLine = "";
    getline(input, FirstLine);

    if (!SetFieldTypes(FirstLine))
    {
        input.close();
        return false;
    }

    string RestOfFileData = "";

    unsigned int RecordID = 0;
    while (getline(input, RestOfFileData))
        _mapRecordsData.insert(pair<unsigned int, string>(RecordID++, RestOfFileData));

    // cerramos el archivo pues ya no lo necesitamos mas
    input.close();

    if (_mapRecordsData.empty())
    {
        printf("ERROR: '%s': No records found.\n", _fileName);
        return false;
    }

    if (!CheckFieldsOfEachRecordAndSaveAllData())
        return false;

    return true;
}

map<unsigned int, string> CSV_Reader::GetFields(string text)
{
    map<unsigned int, string> mapFields;
    ExtractFields(text, mapFields);

    return mapFields;
}

bool CSV_Reader::ExtractFields(string originalText, map<unsigned int, string> &mapFields)
{
    unsigned int fieldID = 0;
    if (originalText.empty())
    {
        mapFields.insert(pair<unsigned int, string>(fieldID++, ""));
        return true;
    }

    string _fieldData = "";
    for (unsigned int x = 0; x < originalText.size(); x++)
    {
        bool isFirstRecord = (x == 0);
        bool isLastRecord = (x + 1) >= originalText.size();

        if (originalText[x] == ',')
        {
            if (isFirstRecord)
            {
                mapFields.insert(pair<unsigned int, string>(fieldID++, ""));
                continue;
            }

            mapFields.insert(pair<unsigned int, string>(fieldID++, _fieldData));
            _fieldData.clear();

            if (isLastRecord)
            {
                mapFields.insert(pair<unsigned int, string>(fieldID++, ""));
                break;
            }

            continue;
        }
        else if (originalText[x] == '"')
        {
            string _stringField = "";

            if (!isFirstRecord && originalText[x - 1] != ',')
            {
                unsigned int min = x;
                unsigned int max = originalText.size() < 30 ? originalText.size() : 30;
                printf("ERROR: '%s': Unexpected start of string in field '%u'\n\tExpected ',' at row %u before '%s'\n",
                    _fileName, mapFields.size() + 1, x + 1, originalText.substr(min, max).c_str());
                return false;
            }

            if (isFirstRecord && isLastRecord)
            {
                printf("WARNING: '%s': Missing \" of string at first field.\n\tIf you want to put an empty text just leave it empty.\n",
                    _fileName);
                mapFields.insert(pair<unsigned int, string>(fieldID++, ""));
                continue;
            }

            if (!isFirstRecord && isLastRecord)
            {
                printf("WARNING: '%s': Missing \" of string at last field (%u).\n\tIf you want to put an empty text just leave it empty.\n",
                    _fileName, mapFields.size() + 1);
                mapFields.insert(pair<unsigned int, string>(fieldID++, ""));
                continue;
            }

            for (unsigned int z = x + 1; z < originalText.size(); z++)
            {
                bool isNotLastStringRecord = (z + 1) < originalText.size();
                bool isLastStringRecord = (z + 1) >= originalText.size();

                if (originalText[z] == '"')
                {
                    if (originalText[z + 1] == '"')
                    {
                        _stringField += '"';
                        z++;
                        continue;
                    }
                    else if (originalText[z + 1] != ',' && isNotLastStringRecord)
                    {
                        int _temp = z - 30;
                        unsigned int min = _temp < 0 ? 0 : _temp;
                        unsigned int max = z - min + 1;
                        printf("ERROR: '%s': Unexpected end of string in field '%u'\n\tExpected ',' at row %u after '%s'\n",
                            _fileName, mapFields.size() + 1, z + 2, originalText.substr(min, max).c_str());
                        return false;
                    }

                    _fieldData = _stringField;
                    x = z;

                    break;
                }
                else if (originalText[z] == '\\')
                {
                    if (originalText[z + 1] == 'n')
                    {
                        z++;
                        _stringField += '\n';
                        continue;
                    }
                    else if (originalText[z + 1] == 'r')
                    {
                        z++;
                        _stringField += '\r';
                        continue;
                    }

                    _stringField += '\\';
                }
                else
                    _stringField += originalText[z];

                if (isLastStringRecord)
                {
                    int _temp = originalText.size() - 30;
                    unsigned int min = _temp < 0 ? 0 : _temp;
                    unsigned int max = originalText.size() - _temp;
                    printf("ERROR: '%s': Unexpected end of line of string in field '%u'\n\tExpected '\"' at row %u before '%s'\n",
                        _fileName, mapFields.size() + 1, originalText.size() + 1, originalText.substr(min, max).c_str());
                    return false;
                }
            }
        }
        else
            _fieldData += originalText[x];

        isLastRecord = (x + 1) >= originalText.size();

        if (isLastRecord)
        {
            mapFields.insert(pair<unsigned int, string>(fieldID++, _fieldData));
            _fieldData.clear();
            break;
        }

        continue;
    }

    return true;
}

bool CSV_Reader::SetFieldTypes(string FirstLine)
{
    if (FirstLine.empty())
    {
        printf("ERROR: '%s': First line can't be empty. Must contain field types\n\tLike: int,uint,float,ufloat,byte,ubyte,string,bool\n\n", _fileName);
        return false;
    }

    auto fieldNames = GetFields(FirstLine);

    for (auto it = fieldNames.begin(); it != fieldNames.end(); ++it)
    {
        if (Shared->CompareTexts(it->second, "string"))
        {
            _fieldTypes.push_back(type_STRING);
            _recordSize += 4;
        }
        else if (Shared->CompareTexts(it->second, "float"))
        {
            _fieldTypes.push_back(type_FLOAT);
            _recordSize += 4;
        }
        else if (Shared->CompareTexts(it->second, "byte"))
        {
            _fieldTypes.push_back(type_BYTE);
            _recordSize += 1;
        }
        else if (Shared->CompareTexts(it->second, "ubyte"))
        {
            _fieldTypes.push_back(type_UBYTE);
            _recordSize += 1;
        }
        else if (Shared->CompareTexts(it->second, "int"))
        {
            _fieldTypes.push_back(type_INT);
            _recordSize += 4;
        }
        else if (Shared->CompareTexts(it->second, "uint"))
        {
            _fieldTypes.push_back(type_UINT);
            _recordSize += 4;
        }
        else if (Shared->CompareTexts(it->second, "bool"))
        {
            _fieldTypes.push_back(type_BOOL);
            _recordSize += 4;
        }
        else if (it->second.empty())
        {
            printf("ERROR: '%s': Name of field '%u' can't be empty.\n", _fileName, it->first + 1);
            return false;
        }
        else
        {
            printf("ERROR: '%s': In Field '%u' Unknown type '%s'.\n", _fileName, it->first + 1, it->second.c_str());
            return false;
        }
    }

    _totalFields = fieldNames.size();

    return true;
}

enumFieldTypes CSV_Reader::GetFieldType(unsigned int currentField)
{
    if (currentField >= _fieldTypes.size())
    {
        printf("ERROR: '%s': This should never happen 'currentField >= _fieldTypes.size()'\n", _fileName);
        return type_NONE;
    }

    switch (_fieldTypes[currentField])
    {
        case type_STRING:   return type_STRING;
        case type_BYTE:     return type_BYTE;
        case type_UBYTE:    return type_UBYTE;
        case type_FLOAT:    return type_FLOAT;
        case type_INT:      return type_INT;
        case type_UINT:     return type_UINT;
        case type_BOOL:     return type_BOOL;
        default:            return type_NONE;
    }
}

const char* CSV_Reader::GetFieldTypeName(enumFieldTypes fieldType)
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

bool CSV_Reader::CheckFieldValue(unsigned int fieldID, enumFieldTypes fieldType, string fieldValue, unsigned int recordID)
{
    bool isFloat = fieldType == type_FLOAT ? true : false;
    bool isBool = fieldType == type_BOOL ? true : false;
    bool isUnsigned = (fieldType == type_UINT || fieldType == type_UBYTE) ? true : false;

    if (fieldValue.empty())
    {
        printf("ERROR: '%s': Field '%u' Type '%s' Line '%u'\n\tCan't be empty. If you want to leave it empty, put value of '0' instead.\n",
            _fileName, fieldID + 1, GetFieldTypeName(fieldType), recordID + 1);

        return false;
    }

    int DotFirst = fieldValue.find('.');
    int DotSecond = fieldValue.rfind('.');

    if (!isFloat && DotFirst != -1)
    {
        printf("ERROR: '%s':Field '%u' Type '%s' Line '%u'\n\tCan't contain dot symbol '.'\n",
            _fileName, fieldID + 1, GetFieldTypeName(fieldType), recordID + 1);
        return false;
    }

    if (isFloat)
    {
        if (DotFirst != -1 && DotSecond != -1 && DotFirst != DotSecond)
        {
            printf("ERROR: '%s': Field '%u' Type '%s' Line '%u'\n\tCan't contains more than one dot symbol '.'\n",
                _fileName, fieldID + 1, GetFieldTypeName(fieldType), recordID + 1);
            return false;
        }

        if (DotFirst == 0 && (fieldValue.size() - 1) == 0)
        {
            printf("ERROR: '%s': Field '%u' Type '%s' Line '%u'\n\tDot symbol '.' can't be the only character in field value.\n",
                _fileName, fieldID + 1, GetFieldTypeName(fieldType), recordID + 1);
            return false;
        }

        if (DotFirst == 0)
        {
            printf("ERROR: '%s': Field '%u' Type '%s' Line '%u'\n\tDot symbol '.' can't be at the start of field value.\n",
                _fileName, fieldID + 1, GetFieldTypeName(fieldType), recordID + 1);
            return false;
        }

        if (DotFirst == (fieldValue.size() - 1))
        {
            printf("ERROR: '%s': Field '%u' Type '%s' Line '%u'\n\tDot symbol '.' can't be the last character in field value.\n",
                _fileName, fieldID + 1, GetFieldTypeName(fieldType), recordID + 1);
            return false;
        }
    }

    int NegativeFirst = fieldValue.find('-');
    int NegativeSecond = fieldValue.rfind('-');

    if ((isBool || isUnsigned) && NegativeFirst != -1)
    {
        printf("ERROR: '%s': Field '%u' Type '%s' Line '%u'\n\tCan't contain negative symbol '-' in field value.\n",
            _fileName, fieldID + 1, GetFieldTypeName(fieldType), recordID + 1);
        return false;
    }

    if (!isBool && !isUnsigned)
    {
        if (NegativeFirst != -1 && NegativeSecond != -1 && NegativeFirst != NegativeSecond)
        {
            printf("ERROR: '%s': Field '%u' Type '%s' Line '%u'\n\tCan't contains more than one negative symbol '-' in field value.\n",
                _fileName, fieldID + 1, GetFieldTypeName(fieldType), recordID + 1);
            return false;
        }

        if (NegativeFirst > 0)
        {
            printf("ERROR: '%s': Field '%u' Type '%s' Line '%u'\n\tNegative symbol '-' only can be at the start of field value.\n",
                _fileName, fieldID + 1, GetFieldTypeName(fieldType), recordID + 1);
            return false;
        }

        if (NegativeFirst == 0 && (fieldValue.size() - 1) == 0)
        {
            printf("ERROR: '%s': Field '%u' Type '%s' Line '%u'\n\tNegative symbol '-' can't be the only character in field value.\n",
                _fileName, fieldID + 1, GetFieldTypeName(fieldType), recordID + 1);
            return false;
        }

        if (NegativeFirst == (fieldValue.size() - 1))
        {
            printf("ERROR: '%s': Field '%u' Type '%s' Line '%u'\n\tNegative symbol '-' can't be the last character in field value.\n",
                _fileName, fieldID + 1, GetFieldTypeName(fieldType), recordID + 1);
            return false;
        }
    }

    for (unsigned int currentChar = 0; currentChar < fieldValue.size(); currentChar++)
    {
        if (isFloat && fieldValue[currentChar] == '.')
            continue;

        if ((fieldValue[currentChar] == '-') || (fieldValue[currentChar] == '3') || (fieldValue[currentChar] == '7') ||
            (fieldValue[currentChar] == '0') || (fieldValue[currentChar] == '4') || (fieldValue[currentChar] == '8') ||
            (fieldValue[currentChar] == '1') || (fieldValue[currentChar] == '5') || (fieldValue[currentChar] == '9') ||
            (fieldValue[currentChar] == '2') || (fieldValue[currentChar] == '6'))
            continue;

        printf("ERROR: '%s': Field '%u' Type '%s' Line '%u'\n\tContains a non numeric value.\n",
            _fileName, fieldID + 1, GetFieldTypeName(fieldType), recordID + 1);

        return false;
    }

    int testValue = atoi(fieldValue.c_str());

    if (fieldType == type_BOOL && (testValue < 0 || testValue > 1))
    {
        printf("ERROR: '%s': Field '%u' Type '%s' Line '%u'\n\tValue can be only '0' or '1'.\n",
            _fileName, fieldID + 1, GetFieldTypeName(fieldType), recordID + 1);
        return false;
    }

    if (fieldType == type_BYTE && (testValue < -127 || testValue > 255))
    {
        printf("ERROR: '%s': Field '%u' Type '%s' Line '%u'\n\tValue can be only between '-127' and '127'.\n",
            _fileName, fieldID + 1, GetFieldTypeName(fieldType), recordID + 1);
        return false;
    }

    if (fieldType == type_UBYTE && (testValue < 0 || testValue > 255))
    {
        printf("ERROR: '%s': Field '%u' Type '%s' Line '%u'\n\tValue can be only between '0' and '255'.\n",
            _fileName, fieldID + 1, GetFieldTypeName(fieldType), recordID + 1);
        return false;
    }

    return true;
}

bool CSV_Reader::CheckFieldsOfEachRecordAndSaveAllData()
{
    unsigned int currentRecord = 0;
    vector<structRecord> Records;
    for (auto itRecords = _mapRecordsData.begin(); itRecords != _mapRecordsData.end(); itRecords++, currentRecord++)
    {
        map<unsigned int, string> fieldsOfCurrentRecord = GetFields(itRecords->second);
        if (fieldsOfCurrentRecord.size() != _totalFields)
        {
            printf("ERROR: '%s': Expected '%u' fields not '%u' fields in line '%u'.\n",
                _fileName, _totalFields, fieldsOfCurrentRecord.size(), itRecords->first + 1);
            return false;
        }

        for (auto itFields = fieldsOfCurrentRecord.begin(); itFields != fieldsOfCurrentRecord.end(); itFields++)
        {
            // solamente para el caso que sea un string entonces simplemente lo ignoramos
            if (GetFieldType(itFields->first) == type_STRING)
                continue;

            // comprobamos si el tipo de dato es correcto para todos fields que deban contener numeros
            if (!CheckFieldValue(itFields->first, GetFieldType(itFields->first), itFields->second, itRecords->first))
                return false;
        }

        // salvamos la informacion desde aqui
        vector<structField> Fields;
        unsigned int currentField = 0;
        for (auto itFields = fieldsOfCurrentRecord.begin(); itFields != fieldsOfCurrentRecord.end(); itFields++, currentField++)
        {
            structField sField;
            sField.ID = currentField;
            sField.Type = GetFieldType(itFields->first);

            if (sField.Type == type_STRING)
            {
                SetUniqueStringTexts(itFields->second);
                sField.Value = Shared->ToStr(GetUniqueStringTextsPosition(itFields->second));
            }
            else
                sField.Value = itFields->second;

            Fields.push_back(sField);
        }

        structRecord Record;
        Record.ID = currentRecord;
        Record.Field = Fields;

        Records.push_back(Record);
    }

    structFileData FileData;
    FileData.Record = Records;
    _fileData.insert(pair<unsigned int, structFileData>(0, FileData));

    _totalRecords = _fileData.size();

    return true;
}