#include "module_csv_reader.h"

CSV_Reader::CSV_Reader(const char *fileName)
{
    _fileName = fileName;

    _recordSize = 0;
    _totalFields = 0;

    _fileData.clear();
    _fieldTypes.clear();
    _mapRecordsData.clear();
}

CSV_Reader::~CSV_Reader()
{
    _fileName = NULL;

    _recordSize = 0;
    _totalFields = 0;

    _fileData.clear();
    _fieldTypes.clear();
    _mapRecordsData.clear();
}

bool CSV_Reader::LoadCSVFile()
{
    Log->WriteLog("Reloading '%s' as text file... ", _fileName);

    // Abrimos el Archivo como solo lectura
    ifstream input(_fileName, ifstream::in);
    if (!input.is_open())
    {
        Log->WriteLogNoTime("FAILED: Unable to reopen file.\n", _fileName);
        Log->WriteLog("\n");
        return false;
    }

    // la primera linea no puede estar vacia nunca
    string TextLine = "";
    getline(input, TextLine);

    // Establecemos el tipo de Fields
    if (!SetFieldTypes(TextLine))
    {
        input.close();
        return false;
    }

    TextLine = "";

    unsigned int RecordID = 0;
    while (getline(input, TextLine))
        _mapRecordsData.insert(pair<unsigned int, string>(RecordID++, TextLine));

    // cerramos el archivo pues ya no lo necesitamos mas
    input.close();

    if (_mapRecordsData.empty())
    {
        Log->WriteLogNoTime("FAILED: No records found.\n");
        Log->WriteLog("\n");
        return false;
    }

    Log->WriteLogNoTime("DONE.\n");

    return true;
}

bool CSV_Reader::ParseFile()
{
    Log->WriteLog("Parsing CSV file... ");

    if (!CheckFieldsOfEachRecordAndSaveAllData())
        return false;

    Log->WriteLogNoTime("DONE.\n");

    return true;
}

void CSV_Reader::PrintResults()
{

}

/*map<unsigned int, string> CSV_Reader::GetFields(string text)
{
    map<unsigned int, string> mapFields;
    ExtractFields(text, mapFields);

    return mapFields;
}*/

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
                Log->WriteLogNoTime("FAILED: Unexpected start of string in field '%u' Expected ',' at row %u before '%s'\n", mapFields.size() + 1, x + 1, originalText.substr(min, max).c_str());
                return false;
            }

            if (isFirstRecord && isLastRecord)
            {
                Log->WriteLogNoTime("FAILED: Missing \" of string at first field. If you want to put an empty text just leave it empty.\n");
                // Puede ser un warning y se agrega el texto y se continua con el for
                // mapFields.insert(pair<unsigned int, string>(fieldID++, ""));
                // continue;
                return false;
            }

            if (!isFirstRecord && isLastRecord)
            {
                Log->WriteLogNoTime("FAILED: Missing \" of string at last field (%u). If you want to put an empty text just leave it empty.\n", mapFields.size() + 1);
                // Puede ser un warning y se agrega el texto y se continua con el for
                // mapFields.insert(pair<unsigned int, string>(fieldID++, ""));
                // continue;
                return false;
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
                        Log->WriteLogNoTime("FAILED: Unexpected end of string in field '%u' Expected ',' at row %u after '%s'\n", mapFields.size() + 1, z + 2, originalText.substr(min, max).c_str());
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
                    Log->WriteLogNoTime("FAILED: Unexpected end of line of string in field '%u' Expected '\"' at row %u before '%s'\n", mapFields.size() + 1, originalText.size() + 1, originalText.substr(min, max).c_str());
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
        Log->WriteLogNoTime("FAILED: First line can't be empty. Must contain field types with comma separated like: int,uint,float,ufloat,byte,ubyte,string,bool\n");
        return false;
    }

    map<unsigned int, string> fieldNames;

    if (!ExtractFields(FirstLine, fieldNames))
        return false;

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
            Log->WriteLogNoTime("FAILED: Name of field '%u' can't be empty.\n", it->first + 1);
            return false;
        }
        else
        {
            Log->WriteLogNoTime("FAILED: In Field '%u' Unknown type '%s'.\n", it->first + 1, it->second.c_str());
            return false;
        }
    }

    // Establecemos el total de Fields conforme a lo leido en la primera linea
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
        Log->WriteLogNoTime("FAILED: Field '%u' Type '%s' Line '%u' Can't be empty. If you want to leave it empty, put value of '0' instead.\n", fieldID + 1, GetFieldTypeName(fieldType), recordID + 1);
        return false;
    }

    int DotFirst = fieldValue.find('.');
    int DotSecond = fieldValue.rfind('.');

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

        if (DotFirst == (fieldValue.size() - 1))
        {
            Log->WriteLogNoTime("FAILED: Field '%u' Type '%s' Line '%u' Dot symbol '.' can't be the last character in field value.\n", fieldID + 1, GetFieldTypeName(fieldType), recordID + 1);
            return false;
        }
    }

    int NegativeFirst = fieldValue.find('-');
    int NegativeSecond = fieldValue.rfind('-');

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

        if (NegativeFirst == (fieldValue.size() - 1))
        {
            Log->WriteLogNoTime("FAILED: Field '%u' Type '%s' Line '%u' Negative symbol '-' can't be the last character in field value.\n", fieldID + 1, GetFieldTypeName(fieldType), recordID + 1);
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

        Log->WriteLogNoTime("FAILED: Field '%u' Type '%s' Line '%u' Contains a non numeric value.\n", fieldID + 1, GetFieldTypeName(fieldType), recordID + 1);

        return false;
    }

    int testValue = atoi(fieldValue.c_str());

    if (fieldType == type_BOOL && (testValue < 0 || testValue > 1))
    {
        Log->WriteLogNoTime("FAILED: Field '%u' Type '%s' Line '%u' Value can be only '0' or '1'.\n", fieldID + 1, GetFieldTypeName(fieldType), recordID + 1);
        return false;
    }

    if (fieldType == type_BYTE && (testValue < -127 || testValue > 255))
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

bool CSV_Reader::CheckFieldsOfEachRecordAndSaveAllData()
{
    unsigned int currentRecord = 0;
    vector<structRecord> Records;
    for (auto itRecords = _mapRecordsData.begin(); itRecords != _mapRecordsData.end(); itRecords++, currentRecord++)
    {
        map<unsigned int, string> fieldsOfCurrentRecord;

        if (!ExtractFields(itRecords->second, fieldsOfCurrentRecord))
            return false;

        if (fieldsOfCurrentRecord.size() != _totalFields)
        {
            Log->WriteLogNoTime("FAILED: Expected '%u' fields not '%u' fields in line '%u'.\n", _totalFields, fieldsOfCurrentRecord.size(), itRecords->first + 1);
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

        // Guardamos la informacion desde aqui
        vector<structField> Fields;
        unsigned int currentField = 0;
        for (auto itFields = fieldsOfCurrentRecord.begin(); itFields != fieldsOfCurrentRecord.end(); itFields++, currentField++)
        {
            /// Guardamos primero los Fields osea las columnas del registro actual
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

        /// Guardamos el Registro actual con todos los Fields previamente almacenados en el vector
        structRecord Record;
        Record.ID = currentRecord;
        Record.Field = Fields;

        Records.push_back(Record);
    }

    // Guardamos todos los Registros del archivo actual al mapa
    structFileData FileData;
    FileData.Record = Records;
    _fileData.insert(pair<string, structFileData>(_fileName, FileData));

    return true;
}