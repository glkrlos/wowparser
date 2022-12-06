#include "module_csv_reader.h"

CSV_Reader::CSV_Reader(const char *fileName)
{
    _fileName = fileName;

    _recordSize = 0;
    _totalFields = 0;
    _totalRecords = 0;

    _fileData.clear();
    _fieldTypes.clear();
    _mapRecordsData.clear();
}

CSV_Reader::~CSV_Reader()
{
    _fileName = NULL;

    _recordSize = 0;
    _totalFields = 0;
    _totalRecords = 0;

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
    for (auto it = _fieldTypes.begin(); it != _fieldTypes.end(); ++it)
    {
        switch (*it)
        {
            case type_FLOAT:    _countFloatFields++; break;
            case type_STRING:   _countStringFields++; break;
            case type_BOOL:     _countBoolFields++; break;
            case type_BYTE:     _countByteFields++; break;
            case type_UBYTE:    _countUByteFields++; break;
            case type_INT:      _countIntFields++; break;
            case type_UINT:     _countUIntFields++; break;
            default: break;
        }
    }

    if (_countFloatFields)
        Log->WriteLog("Total float Fields: '%u'\n", _countFloatFields);

    if (_countStringFields)
        Log->WriteLog("Total string Fields: '%u'\n", _countStringFields);

    if (_countBoolFields)
        Log->WriteLog("Total bool Fields: '%u'\n", _countBoolFields);

    if (_countByteFields)
        Log->WriteLog("Total byte Fields: '%u'\n", _countByteFields);

    if (_countUByteFields)
        Log->WriteLog("Total unsigned byte Fields: '%u'\n", _countUByteFields);

    if (_countIntFields)
        Log->WriteLog("Total int Fields: '%u'\n", _countIntFields);

    if (_countUIntFields)
        Log->WriteLog("Total unsigned int Fields: '%u'\n", _countUIntFields);
}

void CSV_Reader::CreateDBCFile()
{
    string outputFileNameDBC = _fileName;
    outputFileNameDBC.append(".dbc");
    Log->WriteLog("Creating DBC file '%s'... ", outputFileNameDBC.c_str());

    if (_stringTexts.size() != _stringSize)
    {
        Log->WriteLogNoTime("FAILED: Mismatched comparison of strings.\n");
        return;
    }

    FILE *output;
    fopen_s(&output, outputFileNameDBC.c_str(), "wb");
    if (!output)
    {
        Log->WriteLogNoTime("FAILED: Unable to create file.\n");
        return;
    }

    fwrite("WDBC", 4, 1, output);
    fwrite(&_totalRecords, sizeof(_totalRecords), 1, output);
    fwrite(&_totalFields, sizeof(_totalFields), 1, output);
    fwrite(&_recordSize, sizeof(_recordSize), 1, output);
    fwrite(&_stringSize, sizeof(_stringSize), 1, output);

    auto currentFile = _fileData.begin();

    for (auto Records = currentFile->second.Record.begin(); Records != currentFile->second.Record.end(); Records++)
    {
        for (auto Fields = Records->Field.begin(); Fields != Records->Field.end(); Fields++)
        {
            if (Fields->Type == type_FLOAT)
            {
                float value = (float)atof(Fields->Value.c_str());
                fwrite(&value, 4, 1, output);
            }
            else if (Fields->Type == type_BOOL)
            {
                unsigned int value = atoi(Fields->Value.c_str());
                fwrite(&value, 4, 1, output);
            }
            else if (Fields->Type == type_BYTE)
            {
                int value = atoi(Fields->Value.c_str());
                fwrite(&value, 1, 1, output);
            }
            else if (Fields->Type == type_UBYTE)
            {
                unsigned int value = atoi(Fields->Value.c_str());
                fwrite(&value, 1, 1, output);
            }
            else if (Fields->Type == type_INT)
            {
                int value = atoi(Fields->Value.c_str());
                fwrite(&value, 4, 1, output);
            }
            else if (Fields->Type == type_UINT)
            {
                unsigned int value = atoi(Fields->Value.c_str());
                fwrite(&value, 4, 1, output);
            }
            else
            {
                unsigned int value = atoi(Fields->Value.c_str());
                fwrite(&value, 4, 1, output);
            }
        }
    }

    fwrite(_stringTexts.c_str(), _stringTexts.size(), 1, output);

    fclose(output);

    Log->WriteLogNoTime("DONE.\n");

    return;
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
                else if (originalText[x] == '|' &&
                     originalText[x + 1] == '|' &&
                     originalText[x + 2] == '|' &&
                     originalText[x + 3] == '|' &&
                     originalText[x + 4] == 'r' &&
                     originalText[x + 5] == '|' &&
                     originalText[x + 6] == '|' &&
                     originalText[x + 7] == '|' &&
                     originalText[x + 8] == '|')
                {
                    x += 8;
                    _fieldData += '\r';
                    continue;
                }
                else if (originalText[x] == '{' &&
                     originalText[x + 1] == '{' &&
                     originalText[x + 2] == '{' &&
                     originalText[x + 3] == '{' &&
                     originalText[x + 4] == 'n' &&
                     originalText[x + 5] == '}' &&
                     originalText[x + 6] == '}' &&
                     originalText[x + 7] == '}' &&
                     originalText[x + 8] == '}')
                {
                    x += 8;
                    _fieldData += '\n';
                    continue;
                }
                else if (originalText[x] == '[' &&
                     originalText[x + 1] == '[' &&
                     originalText[x + 2] == '[' &&
                     originalText[x + 3] == '[' &&
                     originalText[x + 4] == 't' &&
                     originalText[x + 5] == ']' &&
                     originalText[x + 6] == ']' &&
                     originalText[x + 7] == ']' &&
                     originalText[x + 8] == ']')
                {
                    x += 8;
                    _fieldData += '\t';
                    continue;
                }
                else
                    _fieldData += originalText[x];

                if (isLastStringChar)
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

    _totalRecords = Records.size();

    return true;
}