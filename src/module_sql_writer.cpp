#include "module_sql_writer.h"

bool SQL_Writer::CreateSQLFile()
{
    string outputFileNameCSV = _fileName;
    outputFileNameCSV.append(".sql");
    Log->WriteLog("Creating SQL file '%s'... ", outputFileNameCSV.c_str());

    FILE *output = fopen(outputFileNameCSV.c_str(), "w");
    if (!output)
    {
        Log->WriteLogNoTime("FAILED: Unable to create file.");
        Log->WriteLog("\n");
        return false;
    }

    fprintf(output, "DROP TABLE IF EXISTS `table`;\n");
    fprintf(output, "CREATE TABLE `table` (\n");
    for (unsigned int currentField = 0;  currentField < _fieldTypes.size(); currentField++)
    {
        switch (_fieldTypes[currentField])
        {
            case type_FLOAT:  fprintf(output, "    `field%u` float NOT NULL DEFAULT '0' COMMENT 'float'", currentField); break;
            case type_BOOL:   fprintf(output, "    `field%u` bool NOT NULL DEFAULT '0' COMMENT 'bool'", currentField); break;
            case type_BYTE:   fprintf(output, "    `field%u` smallint(5) NOT NULL DEFAULT '0' COMMENT 'byte'", currentField); break;
            case type_UBYTE:  fprintf(output, "    `field%u` smallint(5) unsigned NOT NULL DEFAULT '0' COMMENT 'unsigned byte'", currentField); break;
            case type_STRING: fprintf(output, "    `field%u` text COMMENT 'string'", currentField); break;
            case type_INT:    fprintf(output, "    `field%u` int(11) NOT NULL DEFAULT '0' COMMENT 'int'", currentField); break;
            case type_UINT:
            default:          fprintf(output, "    `field%u` int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'unsigned int'", currentField); break;
        }

        if (currentField + 1 < _fieldTypes.size())
            fprintf(output, ",");

        fprintf(output, "\n");
    }
    fprintf(output, ") ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='%s';\n\n", outputFileNameCSV.c_str());


    auto currentFile = _savedData.begin();
    unsigned int currentRecord = 0;
    unsigned int maxRows = 0;
    for (auto Records = currentFile->second.Record.begin(); Records != currentFile->second.Record.end(); Records++, currentRecord++)
    {
        if (maxRows == 0)
            fprintf(output, "INSERT INTO `table` VALUES\n");

        maxRows++;

        fprintf(output, "(");
        unsigned int currentField = 0;
        for (auto Fields = Records->Field.begin(); Fields != Records->Field.end(); Fields++, currentField++)
        {
            if (_stringTexts.size() > 1 && Fields->Type == type_STRING)
            {
                unsigned int value = Fields->StringValue;
                if (value)
                {
                    string outText = "\"";
                    for (unsigned int x = value; x < _stringTexts.size(); x++)
                    {
                        if (!_stringTexts[x])
                            break;

                        if (_stringTexts[x] == '"')
                            outText.append("\"");

                        if (_stringTexts[x] == '|' &&
                            _stringTexts[x + 1] == '|' &&
                            _stringTexts[x + 2] == '|' &&
                            _stringTexts[x + 3] == '|' &&
                            _stringTexts[x + 4] == 'r' &&
                            _stringTexts[x + 5] == '|' &&
                            _stringTexts[x + 6] == '|' &&
                            _stringTexts[x + 7] == '|' &&
                            _stringTexts[x + 8] == '|')
                        {
                            x += 8;
                            outText += '\r';
                            continue;
                        }

                        if (_stringTexts[x] == '{' &&
                            _stringTexts[x + 1] == '{' &&
                            _stringTexts[x + 2] == '{' &&
                            _stringTexts[x + 3] == '{' &&
                            _stringTexts[x + 4] == 'n' &&
                            _stringTexts[x + 5] == '}' &&
                            _stringTexts[x + 6] == '}' &&
                            _stringTexts[x + 7] == '}' &&
                            _stringTexts[x + 8] == '}')
                        {
                            x += 8;
                            outText += '\n';
                            continue;
                        }

                        if (_stringTexts[x] == '[' &&
                            _stringTexts[x + 1] == '[' &&
                            _stringTexts[x + 2] == '[' &&
                            _stringTexts[x + 3] == '[' &&
                            _stringTexts[x + 4] == 't' &&
                            _stringTexts[x + 5] == ']' &&
                            _stringTexts[x + 6] == ']' &&
                            _stringTexts[x + 7] == ']' &&
                            _stringTexts[x + 8] == ']')
                        {
                            x += 8;
                            outText += '\t';
                            continue;
                        }
                        outText.append(Shared->ToStr(_stringTexts[x]));
                    }
                    outText.append("\"");
                    fprintf(output, "%s", outText.c_str());
                }
                else
                    fprintf(output, "\"\"");
            }
            else if (Fields->Type == type_FLOAT)
                fprintf(output, "%f", Fields->FloatValue);
            else if (Fields->Type == type_BOOL)
                fprintf(output, "%u", Fields->BoolValue);
            else if (Fields->Type == type_BYTE)
                fprintf(output, "%i", Fields->ByteValue);
            else if (Fields->Type == type_UBYTE)
                fprintf(output, "%u", Fields->UByteValue);
            else if (Fields->Type == type_INT)
                fprintf(output, "%i", Fields->IntValue);
            else if (Fields->Type == type_UINT)
                fprintf(output, "%u", Fields->UIntValue);

            if (currentField + 1 < Records->Field.size())
                fprintf(output, ",");
            else
                fprintf(output, ")");
        }

        if (maxRows == 10000 && (currentRecord + 1 < currentFile->second.Record.size()))
        {
            fprintf(output, ";\n\n");
            maxRows = 0;
            continue;
        }

        if (currentRecord + 1 < currentFile->second.Record.size())
            fprintf(output, ",\n");
        else
            fprintf(output, ";");
    }

    fprintf(output, "\n");
    fclose(output);

    Log->WriteLogNoTime("DONE.\n");

    return true;
}