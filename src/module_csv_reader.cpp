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

/*
 * TODO Implement separator char as option
 */
bool CSV_Reader::ExtractDataFields(vector<map<unsigned int, string>> &newData)
{
    map<unsigned int, string> mapFields;
    unsigned int fieldID = 0;
    unsigned int rowID = 1;
    string currentValue = "";
    bool stillOnString = false;
    unsigned int InitialStringRowID = 0;

    StateCSVFile currentState = StateCSVFile::NoQuotedField;

    unsigned int countFileData = 0;
    for (auto& [id, value] : _fileData)
    {
        countFileData++;
        /// Solo reiniciamos fieldID a 1 cuando el estado esta en NoQuotedField, debido a que como se guardan los datos en un mapa
        /// estos datos se pueden reescribirce cuando es un string activo en el estado por que el key del mapa es único
        if (currentState == StateCSVFile::NoQuotedField)
        {
            fieldID = 0;
        }

        /// Si seguimos en estado de StringField o QuotedOnStringField, entonces agregamos un salto de linea
        if (stillOnString)
        {
            currentValue += "\n";

            /// Guardamos el rowID del elemento anterior antes de sumar con rowID++, para saber desde que linea se inicio el string
            if (InitialStringRowID == 0)
                InitialStringRowID = rowID;
        }

        /// Para cada iteración incrementamos el ID del row
        rowID++;

        unsigned int countCurrentCharacter = 0;
        for (auto currentCharacter: value)
        {
            countCurrentCharacter++;

            switch (currentState)
            {
                case StateCSVFile::NoQuotedField:
                    switch (currentCharacter)
                    {
                        case ',':
                            mapFields.insert(pair<unsigned int, string>(fieldID++, currentValue));
                            currentValue = "";
                            continue;
                        case '"':
                            currentState = StateCSVFile::StringField;
                            continue;
                        case ' ':
                            Log->WriteLogNoTime("FAILED: Unexpected empty space. There are blank space, but it is not a text. If you want to write a text, enclose it in double quotes. If it is an empty text, just leave it empty. If it is a number, it cannot have empty spaces, or simply leave the field empty instead. In field '%u' on line '%u'.\n", mapFields.size() + 1, rowID);
                            return false;
                        default:
                            currentValue += currentCharacter;
                            continue;
                    }
                    break;
                case StateCSVFile::StringField:
                    switch (currentCharacter)
                    {
                        case '"':
                            currentState = StateCSVFile::QuotedOnStringField;
                            continue;
                        default:
                            currentValue += currentCharacter;
                            continue;
                    }
                    break;
                case StateCSVFile::QuotedOnStringField:
                    switch (currentCharacter)
                    {
                        case ',':
                            mapFields.insert(pair<unsigned int, string>(fieldID++, currentValue));
                            currentValue = "";
                            currentState = StateCSVFile::NoQuotedField;
                            continue;
                        case '"':
                            currentValue += currentCharacter;
                            currentState = StateCSVFile::StringField;
                            continue;
                        default:
                            int _temp = countCurrentCharacter - 30;
                            unsigned int min = _temp < 0 ? 0 : _temp;
                            unsigned int max = countCurrentCharacter - min - 1;
                            Log->WriteLogNoTime("FAILED: Unexpected end of string in field '%u' Expected ',' at line %u after '%s'.\n", mapFields.size() + 1, rowID, value.substr(min, max).c_str());
                            return false;
                    }
                    break;
            }
        }

        if (countFileData >= _fileData.size() && currentState == StateCSVFile::StringField && mapFields.size() < _totalFields)
        {
#ifdef DEBUG
            printf("->>>Ya termino %lu, %lu, totalfields: %lu, mapfields %lu, pero esta en StringFiled y es la ultima linea y field\n", countFileData, _fileData.size(), _totalFields, mapFields.size());
#endif
            Log->WriteLogNoTime("FAILED: Unexpected end of file -> Missing \" at the end of line '%u' -> Error started at field '%lu'", rowID, fieldID + 1);

            if (InitialStringRowID)
            {
                Log->WriteLogNoTime(" in line '%u'", InitialStringRowID);
            }

            Log->WriteLogNoTime(".\n");

            return false;
        }
        /// Si ya estamos en el estado de NoQuotedField, entonces no requerimos mas stillOnString si sigue establecido
        /// esto para prevenir agregar un salto de linea en currentValue
        if (stillOnString && currentState == StateCSVFile::NoQuotedField)
        {
            stillOnString = false;
        }

#ifdef DEBUG
        printf("mapFields '%lu' -> _totalFields '%u' -> currentState '%i', StateCSVFile::NoQuotedField '%i'\n", mapFields.size(), _totalFields, currentState, StateCSVFile::NoQuotedField);
#endif

        /// Si currentState es diferente de NoQuotedField y ademas, mapFields size + 1 es menor al total de fields
        /// significa que estamos ante un string aun y continuaremos a la siguiente linea
        if (mapFields.size() + 1 < _totalFields && currentState != StateCSVFile::NoQuotedField)
        {
#ifdef DEBUG
            printf("Pasa por aqui %i, '%s' '%lu'\n", currentState, currentValue.c_str(), mapFields.size());
#endif

            stillOnString = true;
            continue;
        }

        /// Insertamos el ultimo field que quedo despues de la coma final o despues de la ultima comilla, salvo que el estado
        /// sea diferente de NoQuotedField y que ademas aun sea menor a _totalFields
        mapFields.insert(pair<unsigned int, string>(fieldID++, currentValue));
        currentValue = "";

        if ( (currentState != StateCSVFile::QuotedOnStringField || currentState != StateCSVFile::StringField) && mapFields.size() != _totalFields)
        {
            Log->WriteLogNoTime("FAILED: Expected '%u' fields not '%u' fields in line '%u'", _totalFields, mapFields.size(), rowID);

            if (InitialStringRowID)
                Log->WriteLogNoTime(" -> Error started from line '%u' and ended in line '%u'", InitialStringRowID, rowID);

            Log->WriteLogNoTime(".\n");

            return false;
        }

#ifdef DEBUG
        printf("%li %u\n", mapFields.size(), _totalFields);
#endif

        newData.push_back(mapFields);
        mapFields.clear();

#ifdef DEBUG
        printf("Estado actual es: %i < debe ser 0\n", currentState);
        printf("\nCURRENT VALUE: %s\n", currentValue.c_str());
        printf("%s\n", value.c_str());
#endif
    }

#ifdef TEST
    for (auto& test: newData)
    {
        printf("Record:\n");
        for (auto& [id, data]: test)
        {
            printf("%u -> %s\n", id, data.c_str());
        }
    }
#endif
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

    if (!Csv::ExtractFields(FirstLine, fieldNames))
        return false;

    for (auto & fieldName : fieldNames)
    {
        if (Shared::CompareTexts(fieldName.second, "string"))
        {
            _fieldTypes.push_back(type_STRING);
            _recordSize += 4;
        }
        else if (Shared::CompareTexts(fieldName.second, "float"))
        {
            _fieldTypes.push_back(type_FLOAT);
            _recordSize += 4;
        }
        else if (Shared::CompareTexts(fieldName.second, "byte"))
        {
            _fieldTypes.push_back(type_BYTE);
            _recordSize += 1;
        }
        else if (Shared::CompareTexts(fieldName.second, "ubyte"))
        {
            _fieldTypes.push_back(type_UBYTE);
            _recordSize += 1;
        }
        else if (Shared::CompareTexts(fieldName.second, "int"))
        {
            _fieldTypes.push_back(type_INT);
            _recordSize += 4;
        }
        else if (Shared::CompareTexts(fieldName.second, "uint"))
        {
            _fieldTypes.push_back(type_UINT);
            _recordSize += 4;
        }
        else if (Shared::CompareTexts(fieldName.second, "bool"))
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

bool CSV_Reader::CheckFieldsOfEachRecordAndSaveAllData()
{
    vector<map<unsigned int, string>> LoadedData;

    if (!ExtractDataFields(LoadedData))
        return false;

    unsigned int currentRecord = 0;
    vector<structRecord> Records;

    for (auto itRecords = LoadedData.begin(); itRecords != LoadedData.end(); itRecords++, currentRecord++)
    {
        for (auto& [fieldID, recordValue] : *itRecords)
        {
            // solamente para el caso que sea un string entonces simplemente lo ignoramos
            if (GetFieldType(fieldID) == type_STRING)
                continue;

            // comprobamos si el tipo de dato es correcto para todos fields que deban contener numeros
            if (!Csv::CheckFieldValue(fieldID, GetFieldType(fieldID), recordValue, currentRecord))
                return false;
        }

        // Guardamos la informacion desde aqui
        vector<structField> Fields;
        unsigned int currentField = 0;
        for (auto itFields = itRecords->begin(); itFields != itRecords->end(); itFields++, currentField++)
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