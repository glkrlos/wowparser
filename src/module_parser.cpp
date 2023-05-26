#include "module_parser.h"

bool Parser::Load()
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
    //MD5 generatehash;
    //hash = generatehash(_wholeFileData, _fileSize);

    // Para los archivos csv, si fuera solo la palabra "int" serian 3 bytes al menos
    // Para los archivos binarios, debe tener al menos 20 bytes de datos al inicio
    if (((GetFileType() == csvFile || FileIsASCII()) && _fileSize < 3) || (!FileIsASCII() && _fileSize < 20))
    {
        Log->WriteLogNoTime("FAILED: File size is too small. Are you sure is a '%s' file?\n", Shared->GetFileExtensionByFileType(_XMLFileInfo.Type));
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

bool Parser::CheckStructure()
{
    if (GetFileType() == csvFile || FileIsASCII())
    {
        map<unsigned int, string> CSVDataMap;

        unsigned int rowcount = 0;
        string currentLine;
        for (unsigned long x = 0; x < _fileSize; x++)
        {
            char c = static_cast<char>(_wholeFileData[x]);

            if (c == 13)
                continue;
            bool isLastChar = (x + 1) >= _fileSize;

            if (c == '\n')
            {
                if (currentLine.empty())
                {
                    Log->WriteLogNoTime("FAILED: Contains an empty line at Row '%u'.\n", rowcount + 1);
                    Log->WriteLog("\n");
                    return false;
                }

                /// Si el ultimo char es \n entonces decidimos si lo hacemos totalmente estricto
                if (isLastChar)
                {
                    Log->WriteLogNoTime("FAILED: Contains an empty line at Last Row.\n");
                    Log->WriteLog("\n");
                    return false;
                }

                CSVDataMap.insert(pair<unsigned int, string>(rowcount++, currentLine));
                currentLine.clear();
                continue;
            }

            currentLine.append(Shared->ToStr(static_cast<char>(_wholeFileData[x])));

            if (isLastChar)
            {
                CSVDataMap.insert(pair<unsigned int, string>(rowcount++, currentLine));
                currentLine.clear();
                break;
            }
        }

        unique_ptr<CSV_Reader> CSVParser(new CSV_Reader(GetFileName(), CSVDataMap));

        if (!CSVParser->CheckCSV())
            return false;

        if (!CSVParser->CheckFieldsOfEachRecordAndSaveAllData())
            return false;

        /// El resto de valores si los proporciona la clase CSV_Reader
        _totalFields = CSVParser->GetTotalFields();
        _totalRecords = CSVParser->GetTotalRecords();
        _recordSize = CSVParser->GetRecordSize();
        _stringSize = CSVParser->GetStringSize();
        _fieldTypes = CSVParser->GetFieldTypes();
        _stringTexts = CSVParser->GetStringTexts();
        _uniqueStringTexts = CSVParser->GetUniqueStringTexts();
        _savedData = CSVParser->GetExtractedData();
    }
    else
    {
        switch (GetFileTypeByHeader())
        {
            case dbcFile:
            {
                unsigned int HeaderSize = 20;
                _totalRecords = HeaderGetUInt();
                _totalFields = HeaderGetUInt();
                _recordSize = HeaderGetUInt();
                unsigned int ReaderStringSize = HeaderGetUInt();

                unsigned int _dataBytes = _fileSize - HeaderSize - ReaderStringSize;
                unsigned int _stringBytes = _fileSize - HeaderSize - _dataBytes;

                if ((_dataBytes != (_totalRecords * _recordSize)) || !ReaderStringSize || (_stringBytes != ReaderStringSize))
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

                _dataTable = new unsigned char[_dataBytes];
                _dataTable = _wholeFileData + _headerOffset;

                _stringTable = new unsigned char[_stringBytes];
                _stringTable = _wholeFileData + _headerOffset + _dataBytes;

                /// Estableciendo el valor de _stringSize, los datos de _stringTexts y _uniqueStringTexts
                SetStringTextsFromStringTable(_stringBytes);
                break;
            }
            case adbFile:
            {
                unsigned int HeaderSize = 48;
                _totalRecords = HeaderGetUInt();
                _totalFields = HeaderGetUInt();
                _recordSize = HeaderGetUInt();
                unsigned int ReaderStringSize = HeaderGetUInt();
                /*unsigned int TableHash = */HeaderGetUInt();
                /*unsigned int Build = */HeaderGetUInt();
                /*unsigned int unk1 = */HeaderGetUInt();
                /*unsigned int unk2 = */HeaderGetUInt();
                /*unsigned int unk3 = */HeaderGetUInt();
                /*unsigned int LocaleID = */HeaderGetUInt();
                /*unsigned int unk4 = */HeaderGetUInt();

                unsigned int _dataBytes = _fileSize - HeaderSize - ReaderStringSize;
                unsigned int _stringBytes = _fileSize - HeaderSize - _dataBytes;

                if ((_dataBytes != (_totalRecords * _recordSize)) || !ReaderStringSize || (_stringBytes != ReaderStringSize))
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

                _dataTable = new unsigned char[_dataBytes];
                _dataTable = _wholeFileData + _headerOffset;

                // Los archivos de la version 6.x si no tienen strings es 2, sino entonces el numero normal
                if (ReaderStringSize > 2)
                {
                    _stringTable = new unsigned char[_stringBytes];
                    _stringTable = _wholeFileData + _headerOffset + _dataBytes;

                    /// Estableciendo el valor de _stringSize, los datos de _stringTexts y _uniqueStringTexts
                    SetStringTextsFromStringTable(_stringBytes);
                }
                else
                    _stringSize = 1;

                break;
            }
            case db2File:
            {
                unsigned int HeaderSize = 32;
                _totalRecords = HeaderGetUInt();
                _totalFields = HeaderGetUInt();
                _recordSize = HeaderGetUInt();
                unsigned int ReaderStringSize = HeaderGetUInt();
                /*unsigned int tableHash = */HeaderGetUInt();
                unsigned int build = HeaderGetUInt();
                /*unsigned int timestamp_last_written = */HeaderGetUInt();

                unsigned int diff = 0;
                if (build > 12880)
                {
                    HeaderSize += 16;
                    unsigned int min_id = HeaderGetUInt();
                    unsigned int max_id = HeaderGetUInt();
                    /*unsigned int locales = */HeaderGetUInt();
                    /*unsigned int unk1 = */HeaderGetUInt();

                    if (max_id)
                    {
                        unsigned int _diffbase = max_id - min_id + 1;
                        diff = _diffbase * 4 + _diffbase * 2;
                        /// fseek(input, diff * 4 + diff * 2, SEEK_CUR); // diff * 4: an index for rows, diff * 2: a memory allocation bank
                        /// Log->WriteLog("diff: %u", _diffbase * 4 + _diffbase * 2);
                    }
                }

                unsigned int _dataBytes = _fileSize - HeaderSize - ReaderStringSize - diff;
                unsigned int _stringBytes = _fileSize - HeaderSize - _dataBytes - diff;

                if ((_dataBytes != (_totalRecords * _recordSize)) || !ReaderStringSize || (_stringBytes != ReaderStringSize))
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

                _dataTable = new unsigned char[_dataBytes];
                _dataTable = _wholeFileData + HeaderSize + diff;

                _stringTable = new unsigned char[_stringBytes];
                _stringTable = _wholeFileData + HeaderSize + diff + _dataBytes;

                SetStringTextsFromStringTable(_stringBytes);
                break;
            }
            case wdbcreaturecacheFile:
            case wdbgameobjectcacheFile:
            case wdbitemnamecacheFile:
            case wdbitemtextcacheFile:
            case wdbnpccacheFile:
            case wdbpagetextcacheFile:
            case wdbquestcacheFile:
            {
                if (!_XMLFileInfo.FormatedTotalFields || _XMLFileInfo.FormatedFieldTypes.empty() || !_XMLFileInfo.FormatedRecordSize)
                {
                    Log->WriteLogNoTime("FAILED: Unable to parse without specified format.\n");
                    Log->WriteLog("\n");
                    return false;
                }

                if (_XMLFileInfo.FormatedTotalFields != _XMLFileInfo.FormatedFieldTypes.size())
                {
                    Log->WriteLogNoTime("FAILED: Mismatch in total fields.\n");
                    Log->WriteLog("\n");
                    return false;
                }

                unsigned int HeaderSize = 32;

                if (_fileSize < HeaderSize)
                {
                    Log->WriteLogNoTime("FAILED: File size is too small. Are you sure is a '%s' file?\n", Shared->GetFileExtensionByFileType(_XMLFileInfo.Type));
                    Log->WriteLog("\n");
                    return false;
                }

                unsigned int revision = HeaderGetUInt();
                if (revision > 15595)
                {
                    Log->WriteLogNoTime("FAILED: Unable to parse beyond revision 15595.\n");
                    Log->WriteLog("\n");
                    return false;
                }

                /*char locale1 = */HeaderGetChar();
                /*char locale2 = */HeaderGetChar();
                /*char locale3 = */HeaderGetChar();
                /*char locale4 = */HeaderGetChar();
                /*unsigned int maxRecordSize = */HeaderGetUInt();
                /*unsigned int unk1 = */HeaderGetUInt();
                /*unsigned int unk2 = */HeaderGetUInt();

                long currentFileSize = (long)_fileSize - 24;
                bool isFirstRecord = true;

                vector<structRecord> Records;
                unsigned int recordCount = 0;
                while (true)
                {
                    unsigned int entry = 0;
                    int recordSize = 0;

                    if ((currentFileSize -= 8) >= 0)
                    {
                        entry = HeaderGetUInt();
                        recordSize = (int)HeaderGetUInt();

                        if (isFirstRecord && (!entry || !recordSize))
                        {
                            Log->WriteLogNoTime("FAILED: No records found.\n");
                            Log->WriteLog("\n");
                            return false;
                        }
                        else if (!isFirstRecord && (!entry || !recordSize))
                            break;

                        isFirstRecord = false;

                        //printf("Entry: %u\n", entry);
                        //printf("RecordSize: %u\n", recordSize);

                        if ((currentFileSize -= recordSize) >= 0)
                        {
                            vector<structField> Fields;

                            structField sEntry;
                            sEntry.ID = 0;
                            sEntry.Type = type_UINT;
                            sEntry.StringValue = 0;
                            sEntry.FloatValue = 0.0f;
                            sEntry.BoolValue = 0;
                            sEntry.ByteValue = 0;
                            sEntry.UByteValue = 0;
                            sEntry.IntValue = 0;
                            sEntry.UIntValue = entry;
                            Fields.push_back(sEntry);

                            structField sRecordSyze;
                            sRecordSyze.ID = 1;
                            sRecordSyze.Type = type_UINT;
                            sRecordSyze.StringValue = 0;
                            sRecordSyze.FloatValue = 0.0f;
                            sRecordSyze.BoolValue = 0;
                            sRecordSyze.ByteValue = 0;
                            sRecordSyze.UByteValue = 0;
                            sRecordSyze.IntValue = 0;
                            sRecordSyze.UIntValue = recordSize;
                            Fields.push_back(sRecordSyze);

                            auto *currentRecordData = new unsigned char[recordSize];
                            currentRecordData = _wholeFileData + _headerOffset;
                            _headerOffset += recordSize;

                            int currentRecordOffSet = 0;
                            for (unsigned int currentField = 2; currentField < _XMLFileInfo.FormatedFieldTypes.size(); currentField++)
                            {
                                structField sField;
                                sField.ID = currentField;
                                sField.Type = _XMLFileInfo.FormatedFieldTypes[currentField];
                                sField.StringValue = 0;
                                sField.FloatValue = 0.0f;
                                sField.BoolValue = 0;
                                sField.ByteValue = 0;
                                sField.UByteValue = 0;
                                sField.IntValue = 0;
                                sField.UIntValue = 0;

                                if (_XMLFileInfo.FormatedFieldTypes[currentField] == type_FLOAT)
                                {
                                    sField.FloatValue = *reinterpret_cast<float *>(currentRecordData + currentRecordOffSet);
                                    currentRecordOffSet += 4;
                                    recordSize -= 4;
                                }
                                else if (_XMLFileInfo.FormatedFieldTypes[currentField] == type_BOOL)
                                {
                                    sField.BoolValue = *reinterpret_cast<char *>(currentRecordData + currentRecordOffSet);
                                    currentRecordOffSet += 4;
                                    recordSize -= 4;
                                }
                                else if (_XMLFileInfo.FormatedFieldTypes[currentField] == type_BYTE)
                                {
                                    sField.ByteValue = *reinterpret_cast<char *>(currentRecordData + currentRecordOffSet);
                                    currentRecordOffSet += 1;
                                    recordSize -= 1;
                                }
                                else if (_XMLFileInfo.FormatedFieldTypes[currentField] == type_UBYTE)
                                {
                                    sField.UByteValue = *reinterpret_cast<unsigned char *>(currentRecordData + currentRecordOffSet);
                                    currentRecordOffSet += 1;
                                    recordSize -= 1;
                                }
                                else if (_XMLFileInfo.FormatedFieldTypes[currentField] == type_INT)
                                {
                                    sField.IntValue = *reinterpret_cast<int *>(currentRecordData + currentRecordOffSet);
                                    currentRecordOffSet += 4;
                                    recordSize -= 4;
                                }
                                else if (_XMLFileInfo.FormatedFieldTypes[currentField] == type_UINT)
                                {
                                    sField.UIntValue = *reinterpret_cast<unsigned int *>(currentRecordData + currentRecordOffSet);
                                    currentRecordOffSet += 4;
                                    recordSize -= 4;
                                }
                                else /// type_STRING
                                {
                                    string StringValue = reinterpret_cast<char *>(currentRecordData + currentRecordOffSet);
                                    currentRecordOffSet += (int)StringValue.size() + 1;
                                    recordSize -= (int)StringValue.size() + 1;
                                    SetUniqueStringTexts(StringValue);
                                    sField.StringValue = GetUniqueTextPosition(StringValue);
                                }

                                Fields.push_back(sField);
                                if ((recordSize > 0) && ((currentField + 1) >= _XMLFileInfo.FormatedFieldTypes.size()))
                                {
                                    Log->WriteLogNoTime("FAILED: You must read at least '%i' bytes more per record.\n", recordSize);
                                    Log->WriteLog("\n");
                                    return false;
                                }
                                else if ((recordSize < 0) && ((currentField + 1) >= _XMLFileInfo.FormatedFieldTypes.size()))
                                {
                                    Log->WriteLogNoTime("FAILED: Exceeded record size by '%d' bytes.\n", recordSize * -1);
                                    Log->WriteLog("\n");
                                    return false;
                                }
                            }

                            structRecord Record;
                            Record.ID = recordCount;
                            Record.Field = Fields;
                            Records.push_back(Record);
                            recordCount++;
                        }
                        else
                        {
                            Log->WriteLogNoTime("FAILED: Corrupted file.\n");
                            Log->WriteLog("\n");
                            return false;
                        }
                    }
                    else
                    {
                        Log->WriteLogNoTime("FAILED: Unexpected End of file.\n");
                        Log->WriteLog("\n");
                        return false;
                    }
                }
                structFileData FileData;
                FileData.Record = Records;
                _savedData.insert(pair<string, structFileData>(_XMLFileInfo.FileName, FileData));

                _totalFields = _XMLFileInfo.FormatedTotalFields;
                _recordSize = _XMLFileInfo.FormatedRecordSize;
                _totalRecords = Records.size();
                break;
            }
            case wdbitemcacheFile:
                Log->WriteLogNoTime("FAILED: The parse of the WDB itemcache file is temporarily disabled.\n");
                Log->WriteLog("\n");
                return false;
            default:
                Log->WriteLogNoTime("FAILED: Unknown file.\n");
                Log->WriteLog("\n");
                return false;
        }
    }

    return true;
}

void Parser::ParseFile()
{
    if (GetFileType() == csvFile || FileIsASCII())
        ParseCSVFile();
    else
        ParseBinaryFile();
}

bool Parser::ParseBinaryFile()
{
    if (IsPreFormatted())
    {
        Log->WriteLog("Parsing formated file...");

        if (_totalFields != _XMLFileInfo.FormatedTotalFields || _recordSize != _XMLFileInfo.FormatedRecordSize)
        {
            Log->WriteLogNoTime("FAILED: Formated structure mismatch.\n");
            Log->WriteLog("\n");
            return false;
        }

        SetFormatedFieldsOffset();

        Log->WriteLogNoTime("DONE.\n");

        unique_ptr<PrintFileInfo> PrintInfo(new PrintFileInfo(_XMLFileInfo.FormatedFieldTypes, _XMLFileInfo.FormatedTotalFields, _totalRecords, false, hash));

        if (!PrintInfo->PrintResults())
            return false;

        if (_XMLFileInfo.Type != wdbFile)
        {
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
                    sField.Type = _XMLFileInfo.FormatedFieldTypes[currentField];
                    sField.StringValue = 0;
                    sField.FloatValue = 0.0f;
                    sField.BoolValue = 0;
                    sField.ByteValue = 0;
                    sField.UByteValue = 0;
                    sField.IntValue = 0;
                    sField.UIntValue = 0;

                    if (_XMLFileInfo.FormatedFieldTypes[currentField] == type_FLOAT)
                        sField.FloatValue = GetRecord(currentRecord).GetFloat(currentField);
                    else if (_XMLFileInfo.FormatedFieldTypes[currentField] == type_BOOL)
                        sField.BoolValue = GetRecord(currentRecord).GetBool(currentField);
                    else if (_XMLFileInfo.FormatedFieldTypes[currentField] == type_BYTE)
                        sField.ByteValue = GetRecord(currentRecord).GetByte(currentField);
                    else if (_XMLFileInfo.FormatedFieldTypes[currentField] == type_UBYTE)
                        sField.UByteValue = GetRecord(currentRecord).GetUByte(currentField);
                    else if (_XMLFileInfo.FormatedFieldTypes[currentField] == type_INT)
                        sField.IntValue = GetRecord(currentRecord).GetInt(currentField);
                    else if (_XMLFileInfo.FormatedFieldTypes[currentField] == type_UINT)
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
            _savedData.insert(pair<string, structFileData>(_XMLFileInfo.FileName, FileData));
        }

        if (_XMLFileInfo.outputFormats.ToDBC)
        {
            unique_ptr<DBC_Writer> DBCWriter(new DBC_Writer(_totalRecords, _totalFields, _recordSize, _stringSize, _stringTexts, _XMLFileInfo.FileName, _savedData));
            DBCWriter->CreateDBCFile();
        }

        if (_XMLFileInfo.outputFormats.ToCSV)
        {
            unique_ptr<CSV_Writer> CSVWriter(new CSV_Writer(_XMLFileInfo.FileName, _XMLFileInfo.FormatedFieldTypes, _savedData, _stringTexts));
            CSVWriter->CreateCSVFile();
        }

        if (_XMLFileInfo.outputFormats.ToSQL)
        {
            unique_ptr<SQL_Writer> SQLWriter(new SQL_Writer(_XMLFileInfo.FileName, _XMLFileInfo.FormatedFieldTypes, _savedData, _stringTexts));
            SQLWriter->CreateSQLFile();
        }
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

            unique_ptr<PrintFileInfo> PrintInfo(new PrintFileInfo(_fieldTypes, _totalFields, _totalRecords, true, hash));

            if (!PrintInfo->PrintResults())
                return false;

            if (_XMLFileInfo.outputFormats.ToDBC)
            {
                unique_ptr<DBC_Writer> DBCWriter(new DBC_Writer(_totalRecords, _totalFields, _recordSize, _stringSize, _stringTexts, _XMLFileInfo.FileName, _savedData));
                DBCWriter->CreateDBCFile();
            }

            if (_XMLFileInfo.outputFormats.ToCSV)
            {
                unique_ptr<CSV_Writer> CSVWriter(new CSV_Writer(_XMLFileInfo.FileName, _fieldTypes, _savedData, _stringTexts));
                CSVWriter->CreateCSVFile();
            }

            if (_XMLFileInfo.outputFormats.ToSQL)
            {
                unique_ptr<SQL_Writer> SQLWriter(new SQL_Writer(_XMLFileInfo.FileName, _fieldTypes, _savedData, _stringTexts));
                SQLWriter->CreateSQLFile();
            }
        }
    }

    Log->WriteLog("\n");

    return true;
}

bool Parser::ParseCSVFile()
{
    unique_ptr<PrintFileInfo> PrintInfo(new PrintFileInfo(_fieldTypes, _totalFields, _totalRecords, false, hash));

    if (!PrintInfo->PrintResults())
        return false;

    if (_XMLFileInfo.outputFormats.ToDBC)
    {
        unique_ptr<DBC_Writer> DBCWriter(new DBC_Writer(_totalRecords, _totalFields, _recordSize, _stringSize, _stringTexts, _XMLFileInfo.FileName, _savedData));
        DBCWriter->CreateDBCFile();
    }

    if (_XMLFileInfo.outputFormats.ToCSV)
    {
        unique_ptr<CSV_Writer> CSVWriter(new CSV_Writer(_XMLFileInfo.FileName, _fieldTypes, _savedData, _stringTexts));
        CSVWriter->CreateCSVFile();
    }

    if (_XMLFileInfo.outputFormats.ToSQL)
    {
        unique_ptr<SQL_Writer> SQLWriter(new SQL_Writer(_XMLFileInfo.FileName, _fieldTypes, _savedData, _stringTexts));
        SQLWriter->CreateSQLFile();
    }

    Log->WriteLog("\n");

    return true;
}

bool Parser::PredictFieldTypes()
{
    // Establecemos field type NONE y extablecemos en donde empieza cada field para todos los fields
    SetFieldTypesToNONE();
    SetPredictedFieldsOffset();

    // Obtenemos los tipos de Fields
    // 01 - Float System
    for (unsigned int currentField = 0; currentField < _totalFields; currentField++)
    {
        for (unsigned int currentRecord = 0; currentRecord < _totalRecords; currentRecord++)
        {
            float floatValue = GetRecord(currentRecord).GetFloat(currentField);
            if (floatValue > 0)
            {
                string floatStringValue = Shared->ToStr(floatValue);
                int isFloat1 = (int)floatStringValue.find('e');
                int isFloat2 = (int)floatStringValue.find('#');
                int isFloat3 =(int) floatStringValue.find("nan");
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
                int intValue = (int)GetRecord(currentRecord).GetUInt(currentField);
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
    _savedData.insert(pair<string, structFileData>(_XMLFileInfo.FileName, FileData));

    return true;
}