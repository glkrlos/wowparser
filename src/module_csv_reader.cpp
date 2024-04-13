#include "module_csv_reader.h"

CSV_Reader::CSV_Reader(const char *FileName, map<unsigned int, string> FileData)
{
    _fileName = FileName;
    _fileData = std::move(FileData);
}

CSV_Reader::~CSV_Reader()
{
    _fileName = nullptr;
    _fileData.clear();
}

bool CSV_Reader::CheckCSV()
{
    /// Checamos si no esta vacio el mapa con toda la informacion
    auto FirstLine = _fileData.begin();
    if (FirstLine == _fileData.end())
    {
        Log->WriteLogNoTime("FAILED: Unexpected empty data.\n");
        Log->WriteLog("\n");
        return false;
    }

    /// Checamos y establecemos los tipos de fields de la primera linea
    if (!SetFieldTypes(FirstLine->second))
        return false;

    /// Borramos el primer registro pues el mapa se pasaria completo y causaria error
    _fileData.erase(FirstLine);

    /// Checamos si tenemos mas lineas
    if (_fileData.empty())
    {
        Log->WriteLogNoTime("FAILED: No records found.\n");
        Log->WriteLog("\n");
        return false;
    }

    return true;
}

bool CSV_Reader::ExtractFields(string originalText, map<unsigned int, string> &mapFields)
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
        bool isOnlyOneChar = isFirstChar && isLastChar;
        bool isOnlyLastChar = !isFirstChar && isLastChar;

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
        else if (originalText[x] == '"')
        {
            if (!isFirstChar && originalText[x - 1] != ',')
            {
                unsigned int min = x;
                unsigned int max = originalText.size() < 30 ? originalText.size() : 30;
                Log->WriteLogNoTime("FAILED: Unexpected start of string in field '%u' Expected ',' at row %u before '%s'\n", mapFields.size() + 1, x + 1, originalText.substr(min, max).c_str());
                return false;
            }

            if (isOnlyOneChar)
            {
                Log->WriteLogNoTime("FAILED: Missing \" of string at first field. If you want to put an empty text just leave it empty.\n");
                return false;
                // Si despues cambio la forma de mostrar los errores, entonces lo dejo como Warning
                //mapFields.insert(pair<unsigned int, string>(fieldID++, ""));
                //continue;
            }

            if (isOnlyLastChar)
            {
                Log->WriteLogNoTime("FAILED: Missing \" of string at last field (%u). If you want to put an empty text just leave it empty.\n", mapFields.size() + 1);
                return false;
                // Si despues cambio la forma de mostrar los errores, entonces lo dejo como Warning
                //mapFields.insert(pair<unsigned int, string>(fieldID++, ""));
                //continue;
            }

            x++;
            for (; x < originalText.size(); x++)
            {
                bool isNotLastStringChar = (x + 1) < originalText.size();
                bool isLastStringChar = (x + 1) >= originalText.size();

                /// Si hay una comilla pueden ocurrir dos cosas
                if (originalText[x] == '"')
                {
                    /// Si el siguiente caracter es otra comilla, siginifica que siguiente solo es comilla doble
                    if (originalText[x + 1] == '"')
                    {
                        _fieldData += '"';
                        x++;
                        continue;
                    }
                    /// Si el siguiente caracter no es coma y no es el ultimo record, entonces se vuelve invalida la terminacion del string
                    else if (originalText[x + 1] != ',' && isNotLastStringChar)
                    {
                        int _temp = x - 30;
                        unsigned int min = _temp < 0 ? 0 : _temp;
                        unsigned int max = x - min + 1;
                        Log->WriteLogNoTime("FAILED: Unexpected end of string in field '%u' Expected ',' at row %u after '%s'\n", mapFields.size() + 1, x + 2, originalText.substr(min, max).c_str());
                        return false;
                    }

                    break;
                }
                else
                    _fieldData += originalText[x];

                if (isLastStringChar)
                {
                    int _temp = (int)originalText.size() - 30;
                    unsigned int min = _temp < 0 ? 0 : _temp;
                    unsigned int max = originalText.size() - _temp;
                    Log->WriteLogNoTime("FAILED: Unexpected end of line of string in field '%u' Expected '\"' at row %u before '%s'\n", mapFields.size() + 1, originalText.size() + 1, originalText.substr(min, max).c_str());
                    return false;
                }
            }
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

bool CSV_Reader::SetFieldTypes(const string& FirstLine)
{
    if (FirstLine.empty())
    {
        Log->WriteLogNoTime("FAILED: First line can't be empty. Must contain field types with comma separated like: int,uint,float,ufloat,byte,ubyte,string,bool\n");
        return false;
    }

    map<unsigned int, string> fieldNames;

    if (!ExtractFields(FirstLine, fieldNames))
        return false;

    for (auto & fieldName : fieldNames)
    {
        if (Shared->CompareTexts(fieldName.second, "string"))
        {
            _fieldTypes.push_back(type_STRING);
            _recordSize += 4;
        }
        else if (Shared->CompareTexts(fieldName.second, "float"))
        {
            _fieldTypes.push_back(type_FLOAT);
            _recordSize += 4;
        }
        else if (Shared->CompareTexts(fieldName.second, "byte"))
        {
            _fieldTypes.push_back(type_BYTE);
            _recordSize += 1;
        }
        else if (Shared->CompareTexts(fieldName.second, "ubyte"))
        {
            _fieldTypes.push_back(type_UBYTE);
            _recordSize += 1;
        }
        else if (Shared->CompareTexts(fieldName.second, "int"))
        {
            _fieldTypes.push_back(type_INT);
            _recordSize += 4;
        }
        else if (Shared->CompareTexts(fieldName.second, "uint"))
        {
            _fieldTypes.push_back(type_UINT);
            _recordSize += 4;
        }
        else if (Shared->CompareTexts(fieldName.second, "bool"))
        {
            _fieldTypes.push_back(type_BOOL);
            _recordSize += 4;
        }
        else if (fieldName.second.empty())
        {
            Log->WriteLogNoTime("FAILED: Name of field '%u' can't be empty.\n", fieldName.first + 1);
            return false;
        }
        else
        {
            Log->WriteLogNoTime("FAILED: In Field '%u' Unknown type '%s'.\n", fieldName.first + 1, fieldName.second.c_str());
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

bool CSV_Reader::CheckFieldsOfEachRecordAndSaveAllData()
{
    unsigned int currentRecord = 0;
    vector<structRecord> Records;
    for (auto itRecords = _fileData.begin(); itRecords != _fileData.end(); itRecords++, currentRecord++)
    {
        map<unsigned int, string> fieldsOfCurrentRecord;

        if (!ExtractFields(itRecords->second, fieldsOfCurrentRecord))
            return false;

        if (fieldsOfCurrentRecord.size() != _totalFields)
        {
            Log->WriteLogNoTime("FAILED: Expected '%u' fields not '%u' fields in line '%u'.\n", _totalFields, fieldsOfCurrentRecord.size(), itRecords->first + 1);
            return false;
        }

        for (auto & itFields : fieldsOfCurrentRecord)
        {
            // solamente para el caso que sea un string entonces simplemente lo ignoramos
            if (GetFieldType(itFields.first) == type_STRING)
                continue;

            // comprobamos si el tipo de dato es correcto para todos fields que deban contener numeros
            if (!CheckFieldValue(itFields.first, GetFieldType(itFields.first), itFields.second, itRecords->first))
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
            sField.StringValue = 0;
            sField.FloatValue = 0.0f;
            sField.BoolValue = 0;
            sField.ByteValue = 0;
            sField.UByteValue = 0;
            sField.IntValue = 0;
            sField.UIntValue = 0;

            if (sField.Type == type_FLOAT)
                sField.FloatValue = (float)atof(itFields->second.c_str());
            else if (sField.Type == type_BOOL)
                sField.BoolValue = atoi(itFields->second.c_str());
            else if (sField.Type == type_BYTE)
                sField.ByteValue = atoi(itFields->second.c_str());
            else if (sField.Type == type_UBYTE)
                sField.UByteValue = atoi(itFields->second.c_str());
            else if (sField.Type == type_INT)
                sField.IntValue = atoi(itFields->second.c_str());
            else if (sField.Type == type_UINT)
                sField.UIntValue = atoi(itFields->second.c_str());
            else /// type_STRING
            {
                SetUniqueStringTexts(itFields->second);
                sField.StringValue = GetUniqueTextPosition(itFields->second);
            }

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
    _savedData.insert(pair<string, structFileData>(_fileName, FileData));

    _totalRecords = Records.size();

    return true;
}