#include "module_parser.h"

bool module_parser::Load()
{
    _inputFile = fopen(GetFileName(), "rb");
    if (!_inputFile)
    {
        Log->WriteLog("ERROR: Can't open file '%s'.\n", GetFileName());
        return false;
    }

    Log->WriteLog("Reading file '%s'... ", GetFileName());

    fseek(_inputFile, 0, SEEK_END);
    _fileSize = ftell(_inputFile);

    if (!_fileSize)
    {
        Log->WriteLogNoTime("FAILED: Empty File.\n");
        Log->WriteLog("\n");
        return false;
    }

    rewind(_inputFile);

    _wholeFileData = new unsigned char[_fileSize];
    if (fread(_wholeFileData, _fileSize, 1, _inputFile) != 1)
    {
        Log->WriteLogNoTime("FAILED: Unable to read file.\n");
        Log->WriteLog("\n");
        return false;
    }

    /// Cerramos el archivo pues ya no lo necesitamos mas
    if (_inputFile)
        fclose(_inputFile);

    if (NullPoniterToData())
    {
        Log->WriteLogNoTime("FATAL: DATA_READ_ERROR: null Pointer to file data. Report this to fix it.\n");
        Log->WriteLog("\n");
        return false;
    }

    /// Generamos el hash del archivo
    MD5 generatehash;
    hash = generatehash(_wholeFileData, _fileSize);

    // Para los archivos csv, si fuera solo la palabra "int" serian 3 bytes al menos
    // Para los archivos binarios, debe tener al menos 20 bytes de datos al inicio
    if (((GetFileType() == csvFile || FileIsASCII()) && _fileSize < 3) || (!FileIsASCII() && _fileSize < 20))
    {
        Log->WriteLogNoTime("FAILED: File size is too small. Are you sure is a '%s' file?\n", Shared->GetFileExtensionByFileType(_fileType));
        Log->WriteLog("\n");
        return false;
    }

    if ((GetFileType() == csvFile || FileIsASCII()))
    {
        Log->WriteLogNoTime("DONE.\n");
        Log->WriteLog("Parsing file... ");
    }

    if (!CheckStructure())
        return false;

    Log->WriteLogNoTime("DONE.\n");

    return true;
}

bool module_parser::CheckStructure()
{
    if (GetFileType() == csvFile || FileIsASCII())
    {
        map<unsigned int, string> CSVDataMap;

        unsigned int rowcount = 0;
        string currentLine = "";
        for (unsigned long x = 0; x < _fileSize; x++)
        {
            char c = static_cast<char>(_wholeFileData[x]);

            if (c == 13)
                continue;

            bool isLastChar = (x + 1) >= _fileSize;

            /// Si es nueva linea entonces
            if (c == '\n')
            {
                /// Si es la ultima letra y no esta vacio el string, agregamos y salimos
                if (isLastChar && !currentLine.empty())
                {
                    rowcount++;
                    CSVDataMap.insert(pair<unsigned int, string>(rowcount, currentLine));
                    break;
                }
                /// Si no es la ultima letra y la linea esta vacia, entonces es error
                else if (!isLastChar && currentLine.empty())
                {
                    Log->WriteLogNoTime("FAILED: Contains an empty line at Row '%u'.\n", rowcount);
                    Log->WriteLog("\n");
                    return false;
                }
                /// Cualquier otra cosa significa que ni es la ultima letra y ni esta vacio el string, agregamos y continuamos
                rowcount++;
                CSVDataMap.insert(pair<unsigned int, string>(rowcount, currentLine));
                currentLine.clear();
                continue;
            }
            /// Si es la ultima letra
            else if (isLastChar)
            {
                /// Si la linea no esta vacia, agregamos y salimos
                if (!currentLine.empty())
                {
                    rowcount++;
                    CSVDataMap.insert(pair<unsigned int, string>(rowcount, currentLine));
                    break;
                }
                /// Por consiguiente esta vacio el string y como es la ultima letra no necesitamos seguir
                break;
            }

            currentLine.append(Shared->ToStr(static_cast<char>(_wholeFileData[x])));
        }

        auto_ptr<CSV_Reader> CSVParser(new CSV_Reader(GetFileName(), CSVDataMap));

        if (!CSVParser->CheckCSV())
            return false;

        if (!CSVParser->CheckFieldsOfEachRecordAndSaveAllData())
            return false;

        /// Header para archivos CSV es siempre 0 pues no es un archivo binario
        _headerSize = 0;
        /// El resto de valores si los proporciona la clase CSV_Reader
        _totalFields = CSVParser->GetTotalTotalFields();
        _totalRecords = CSVParser->GetTotalTotalRecords();
        _recordSize = CSVParser->GetTotalRecordSize();
        _stringSize = CSVParser->GetStringSize();
        _fieldTypes = CSVParser->GetFieldTypes();
        _stringTexts = CSVParser->GetStringTexts();
        _uniqueStringTexts = CSVParser->GetUniqueStringTexts();
        _extractedData = CSVParser->GetExtractedData();
    }
    else
    {
        switch (GetFileTypeByHeader())
        {
            case dbcFile:
            case adbFile:
            {
                _headerSize = 20;
                _totalRecords = HeaderGetUInt();
                _totalFields = HeaderGetUInt();
                _recordSize = HeaderGetUInt();
                unsigned int ReadedStringSize = HeaderGetUInt();

                unsigned int _dataBytes = _fileSize - _headerSize - ReadedStringSize;
                unsigned int _stringBytes = _fileSize - _headerSize - _dataBytes;

                if ((_dataBytes != (_totalRecords * _recordSize)) || !ReadedStringSize || (_stringBytes != ReadedStringSize))
                {
                    Log->WriteLogNoTime("FAILED: Structure is damaged.\n");
                    Log->WriteLog("\n");
                    return false;
                }

                if (!_totalRecords || !_totalFields || !_recordSize)
                {
                    Log->WriteLogNoTime("FAILED: No records found.\n");
                    Log->WriteLog("\n");
                    return false;
                }

                _dataBytes = _fileSize - _headerSize - ReadedStringSize;
                _dataTable = new unsigned char [_dataBytes];
                _dataTable = _wholeFileData + _headerOffset;

                _stringBytes = _fileSize - _headerSize - _dataBytes;
                _stringTable = new unsigned char[_stringBytes];
                _stringTable = _wholeFileData + _headerOffset + _dataBytes;

                /// Estableciendo el valor de _stringSize, los datos de _stringTexts y _uniqueStringTexts
                SetUniqueStringTextsFromStringTable(_stringBytes);
                break;
            }
            case db2File:
                Log->WriteLogNoTime("FAILED: Temporarily disabled the parse of DB2 files.\n");
                Log->WriteLog("\n");
                return false;
                /// 32 bytes del header o 48 bytes si el build > 12880
                //char header[4];             // WDB2 db2
                //unsigned int totalRecords;
                //unsigned int totalFields;
                //unsigned int recordSize;
                //unsigned int stringSize;
                //unsigned int tableHash;
                //unsigned int build;
                //unsigned int unk1;

                /// > 12880
                /// int diff = maxIndexDB2 - unk2DB2 + 1;
                /// fseek(input, diff * 4 + diff * 2, SEEK_CUR); // diff * 4: an index for rows, diff * 2: a memory allocation bank
                //unsigned int unk2;
                //unsigned int maxIndex;
                //unsigned int locales;
                //unsigned int unk3;
                break;
            case wdbitemcacheFile:
            case wdbcreaturecacheFile:
            case wdbgameobjectcacheFile:
            case wdbitemnamecacheFile:
            case wdbitemtextcacheFile:
            case wdbnpccacheFile:
            case wdbpagetextcacheFile:
            case wdbquestcacheFile:
                Log->WriteLogNoTime("FAILED: Temporarily disabled the parse of WDB files.\n");
                Log->WriteLog("\n");
                return false;
                /// 24 bytes del header + 8 bytes del primer record y su el tamaño del record
                //char header[4];
                //unsigned int revision;
                //char locale[4];
                //unsigned int maxRecordSize;
                //unsigned int unk1;
                //unsigned int unk2;
                /*
                    Para cada registro:
                    unsigned int entry;
                    unsigned int recordSize;
                    unsigned char *restOfrecord; <- aqui hay que saber el formato para leerlo

                    BDIW itemcache.wdb -> se abre de forma especial por que dependen de unos bytes las veces que lee otros bytes
                    BOMW creaturecache.wdb
                    BOGW gameobjectcache.wdb
                    BDNW itemnamecache.wdb
                    XTIW itemtextcache.wdb
                    CPNW npccache.wdb
                    XTPW pagetextcache.wdb
                    TSQW questcache.wdb
                */
                break;
            default:
                Log->WriteLogNoTime("FAILED: Unknown file.\n");
                Log->WriteLog("\n");
                return false;
        }
    }

    return true;
}

void module_parser::ParseFile()
{
    if (GetFileType() == csvFile || FileIsASCII())
        ParseCSVFile();
    else
        ParseBinaryFile();
}

bool module_parser::ParseBinaryFile()
{
    if (IsPreFormatted())
    {
        Log->WriteLog("Parsing formated file...");

        if (_totalFields != _formatedTotalFields || _recordSize != _formatedRecordSize)
        {
            Log->WriteLogNoTime("FAILED: Formated structure mismatch.\n");
            Log->WriteLog("\n");
            return false;
        }

        SetFieldsOffset(_formatedFieldTypes);

        Log->WriteLogNoTime("DONE.\n");

        auto_ptr<PrintFileInfo> PrintInfo(new PrintFileInfo(_formatedFieldTypes, _formatedTotalFields, _totalRecords, false, hash));

        if (!PrintInfo->PrintResults())
            return false;

        // Guardamos la informacion al mapa map<string, structFileData> _extractedData
        unsigned int currentRecordID = 0;
        vector<structRecord> Records;

        for (unsigned int currentRecord = 0; currentRecord < _totalRecords; currentRecord++, currentRecordID++)
        {
            vector<structField> Fields;

            for (unsigned int currentField = 0; currentField < _totalFields; currentField++)
            {
                structField sField;
                sField.ID = currentField;
                sField.Type = _formatedFieldTypes[currentField];
                sField.StringValue = 0;
                sField.FloatValue = 0.0f;
                sField.BoolValue = 0;
                sField.ByteValue = 0;
                sField.UByteValue = 0;
                sField.IntValue = 0;
                sField.UIntValue = 0;

                if (_formatedFieldTypes[currentField] == type_FLOAT)
                    sField.FloatValue = GetRecord(currentRecord).GetFloat(currentField);
                else if (_formatedFieldTypes[currentField] == type_BOOL)
                    sField.BoolValue = GetRecord(currentRecord).GetBool(currentField);
                else if (_formatedFieldTypes[currentField] == type_BYTE)
                    sField.ByteValue = GetRecord(currentRecord).GetByte(currentField);
                else if (_formatedFieldTypes[currentField] == type_UBYTE)
                    sField.UByteValue = GetRecord(currentRecord).GetUByte(currentField);
                else if (_formatedFieldTypes[currentField] == type_INT)
                    sField.IntValue = GetRecord(currentRecord).GetInt(currentField);
                else if (_formatedFieldTypes[currentField] == type_UINT)
                    sField.UIntValue = GetRecord(currentRecord).GetUInt(currentField);
                else // type_STRING
                    sField.StringValue = GetRecord(currentRecord).GetUInt(currentField);

                Fields.push_back(sField);
            }

            structRecord Record;
            Record.ID = currentRecordID;
            Record.Field = Fields;

            Records.push_back(Record);
        }

        structFileData FileData;
        FileData.Record = Records;
        _extractedData.insert(pair<string, structFileData>(_fileName, FileData));

        //auto_ptr<DBC_Writer> DBCWriter(new DBC_Writer(_totalRecords, _totalFields, _recordSize, _stringSize, _stringTexts, _fileName, _extractedData));
        //DBCWriter->CreateDBCFile();
        auto_ptr<CSV_Writer> CSVWriter(new CSV_Writer(_fileName, _formatedFieldTypes, _extractedData, _stringTexts));
        CSVWriter->CreateCSVFile();
    }
    else
    {
        Log->WriteLog("Predicting field types...");

        if (_recordSize / 4 != _totalFields)
        {
            Log->WriteLogNoTime("FAILED: Not supported byte packed format.\n");
            Log->WriteLog("\n");
            return false;
        }

        if (PredictFieldTypes())
        {
            Log->WriteLogNoTime("DONE.\n");

            auto_ptr<PrintFileInfo> PrintInfo(new PrintFileInfo(_fieldTypes, _totalFields, _totalRecords, true, hash));

            if (!PrintInfo->PrintResults())
                return false;

            //auto_ptr<DBC_Writer> DBCWriter(new DBC_Writer(_totalRecords, _totalFields, _recordSize, _stringSize, _stringTexts, _fileName, _extractedData));
            //DBCWriter->CreateDBCFile();
            auto_ptr<CSV_Writer> CSVWriter(new CSV_Writer(_fileName, _fieldTypes, _extractedData, _stringTexts));
            CSVWriter->CreateCSVFile();
        }
    }

    Log->WriteLog("\n");

    return true;
}

bool module_parser::ParseCSVFile()
{
    auto_ptr<PrintFileInfo> PrintInfo(new PrintFileInfo(_fieldTypes, _totalFields, _totalRecords, false, hash));

    if (!PrintInfo->PrintResults())
        return false;

    auto_ptr<DBC_Writer> DBCWriter(new DBC_Writer(_totalRecords, _totalFields, _recordSize, _stringSize, _stringTexts, _fileName, _extractedData));
    DBCWriter->CreateDBCFile();

    Log->WriteLog("\n");

    return true;
}

bool module_parser::PredictFieldTypes()
{
    // Establecemos field type NONE y extablecemos en donde empieza cada field para todos los fields
    SetFieldTypesToNONE();
    SetFieldsOffset(_fieldTypes);

    // Obtenemos los tipos de Fields
    // 01 - Float System
    for (unsigned int currentField = 0; currentField < _totalFields; currentField++)
    {
        for (unsigned int currentRecord = 0; currentRecord < _totalRecords; currentRecord++)
        {
            float floatValue = GetRecord(currentRecord).GetFloat(currentField);
            if (floatValue)
            {
                string floatStringValue = Shared->ToStr(floatValue);
                int isFloat1 = floatStringValue.find("e");
                int isFloat2 = floatStringValue.find("#");
                int isFloat3 = floatStringValue.find("-nan");
                if (isFloat1 != -1 || isFloat2 != -1 || isFloat3 != -1)
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

    // 03 - String System
    if (_stringSize > 1)
    {
        for (unsigned int currentField = 0; currentField < _totalFields; currentField++)
        {
            if (_fieldTypes[currentField] == type_FLOAT || _fieldTypes[currentField] == type_BOOL)
                continue;

            for (unsigned int currentRecord = 0; currentRecord < _totalRecords; currentRecord++)
            {
                int intValue = GetRecord(currentRecord).GetUInt(currentField);
                if (intValue < 0 || intValue >= int(_stringSize) || (intValue > 0 && _stringTable[intValue - 1]))
                {
                    _fieldTypes[currentField] = type_INT;
                    break;
                }

                _fieldTypes[currentField] = type_STRING;
            }
        }

        /// Para los casos cuando el valor fue 1 y obviamente se quedaron en BOOL, hay que rectificarlos solo si el total de strings es diferente al origen
        map<unsigned int, int> TotalTextsPredicted;
        for (unsigned int currentField = 0; currentField < _totalFields; currentField++)
        {
            if (_fieldTypes[currentField] != type_STRING)
                continue;

            for (unsigned int currentRecord = 0; currentRecord < _totalRecords; currentRecord++)
            {
                unsigned int value = GetRecord(currentRecord).GetUInt(currentField);

                if (!value)
                    continue;

                auto it = TotalTextsPredicted.find(value);

                if (it != TotalTextsPredicted.end())
                    continue;

                TotalTextsPredicted.insert(pair<unsigned int, int>(value, 0));
            }
        }

        /// Si no hubo prediccion de strings entonces intentamos buscas mas a fondo
        if (TotalTextsPredicted.size() != _uniqueStringTexts.size())
        {
            /// Volvemos a checar los bool ya que nos faltan strings por realocar, solo se toman encuenta los bool = 1
            unsigned int contamosbool = 0;
            for (unsigned int currentField = 0; currentField < _totalFields; currentField++)
            {
                if (_fieldTypes[currentField] != type_BOOL)
                    continue;

                for (unsigned int currentRecord = 0; currentRecord < _totalRecords; currentRecord++)
                {
                    unsigned int value = GetRecord(currentRecord).GetUInt(currentField);
                    if (value == 1)
                        contamosbool++;
                }
            }

            /// Si la suma total de contamosbool + totaltextpredicted = allstringsintable, siginifica que los strings faltantes fueron contados como bool
            if ((TotalTextsPredicted.size() + contamosbool) == _uniqueStringTexts.size())
            {
                for (unsigned int currentField = 0; currentField < _totalFields; currentField++)
                {
                    if (_fieldTypes[currentField] != type_BOOL)
                        continue;

                    for (unsigned int currentRecord = 0; currentRecord < _totalRecords; currentRecord++)
                    {
                        unsigned int value = GetRecord(currentRecord).GetUInt(currentField);
                        if (value == 1)
                            _fieldTypes[currentField] = type_STRING;
                    }
                }
            }
            else if ((TotalTextsPredicted.size() + contamosbool) < _uniqueStringTexts.size())
            {
                Log->WriteLogNoTime("FAILED: Unable to predict one or more string fields.\n");
                return false;
            }

            /// Y si no se predijeron strings, entonces vamos a sacar la diferencia y buscar todos los strings restantes
            else
            {
                unsigned int stringsFaltantes = (TotalTextsPredicted.size() + contamosbool) - _uniqueStringTexts.size();
                //Log->WriteLog("\nStrings Actuales: %u, Totales: %u, Faltantes: %u, Bool Contados: %u\n", TotalTextsPredicted.size(), _uniqueStringTexts.size(), stringsFaltantes, contamosbool);

                /// Si solo hay un bool, entonces por default es el que falta
                if (contamosbool > 1)
                {
                    /// Aqui omitimos la primer coincidencia y cambiamos a string hasta que no falten strings por predecir
                    for (unsigned int currentField = 0; currentField < _totalFields; currentField++)
                    {
                        if (!stringsFaltantes)
                            break;

                        // Omitimos el primer field pues se supone que hay mas de 1 bool que debe ser string
                        if (_fieldTypes[currentField] != type_BOOL || currentField == 0)
                            continue;

                        for (unsigned int currentRecord = 0; currentRecord < _totalRecords; currentRecord++)
                        {
                            if (!stringsFaltantes)
                                break;

                            unsigned int value = GetRecord(currentRecord).GetUInt(currentField);
                            if (value == 1)
                            {
                                _fieldTypes[currentField] = type_STRING;
                                stringsFaltantes--;
                            }
                        }
                    }
                }
                else if (contamosbool == 1)
                {
                    /// Aqui establecemos el unico bool a tipo string
                    for (unsigned int currentField = 0; currentField < _totalFields; currentField++)
                    {
                        if (_fieldTypes[currentField] != type_BOOL)
                            continue;

                        for (unsigned int currentRecord = 0; currentRecord < _totalRecords; currentRecord++)
                        {
                            unsigned int value = GetRecord(currentRecord).GetUInt(currentField);
                            if (value == 1)
                                _fieldTypes[currentField] = type_STRING;
                        }
                    }
                }
                else
                {
                    /// Nada que hacer, simplemente ignorarlo
                    //Log->WriteLogNoTime("FAILED: contamosbool <= 0: Unable to predict one or more string fields.\n");
                    //return false;
                }
            }
        }
    }

    // 04 - Unsigned/Signed Int System
    for (unsigned int currentField = 0; currentField < _totalFields; currentField++)
    {
        if (_fieldTypes[currentField] == type_FLOAT || _fieldTypes[currentField] == type_BOOL || _fieldTypes[currentField] == type_STRING)
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

    // Guardamos la informacion al mapa map<string, structFileData> _extractedData
    unsigned int currentRecordID = 0;
    vector<structRecord> Records;

    for (unsigned int currentRecord = 0; currentRecord < _totalRecords; currentRecord++, currentRecordID++)
    {
        vector<structField> Fields;

        for (unsigned int currentField = 0; currentField < _totalFields; currentField++)
        {
            structField sField;
            sField.ID = currentField;
            sField.Type = _fieldTypes[currentField];
            sField.StringValue = 0;
            sField.FloatValue = 0.0f;
            sField.BoolValue = 0;
            sField.ByteValue = 0;
            sField.UByteValue = 0;
            sField.IntValue = 0;
            sField.UIntValue = 0;

            if (_fieldTypes[currentField] == type_FLOAT)
                sField.FloatValue = GetRecord(currentRecord).GetFloat(currentField);
            else if (_fieldTypes[currentField] == type_BOOL)
                sField.BoolValue = GetRecord(currentRecord).GetBool(currentField);
            else if (_fieldTypes[currentField] == type_BYTE)
                sField.ByteValue = GetRecord(currentRecord).GetByte(currentField);
            else if (_fieldTypes[currentField] == type_UBYTE)
                sField.UByteValue = GetRecord(currentRecord).GetUByte(currentField);
            else if (_fieldTypes[currentField] == type_INT)
                sField.IntValue = GetRecord(currentRecord).GetInt(currentField);
            else if (_fieldTypes[currentField] == type_UINT)
                sField.UIntValue = GetRecord(currentRecord).GetUInt(currentField);
            else // type_STRING
                sField.StringValue = GetRecord(currentRecord).GetUInt(currentField);

            Fields.push_back(sField);
        }

        structRecord Record;
        Record.ID = currentRecordID;
        Record.Field = Fields;

        Records.push_back(Record);
    }

    structFileData FileData;
    FileData.Record = Records;
    _extractedData.insert(pair<string, structFileData>(_fileName, FileData));

    return true;
}

void module_parser::CheckHeadersAndDataConsistencyOfAllFilesAdded()
{
    ProgressBar bar(_ListOfAllFilesToParse.size());

    for (auto CurrentFileName = _ListOfAllFilesToParse.begin(); CurrentFileName != _ListOfAllFilesToParse.end(); CurrentFileName++)
    {
        bar.step(CurrentFileName->first.c_str());

        auto_ptr<module_parser> Parser(new module_parser(CurrentFileName->second));
        if (Parser->Load())
            Parser->ParseFile();

    }
}