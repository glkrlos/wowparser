#include "module_csv_writer.h"

bool CSV_Writer::CreateCSVFile()
{
    string outputFileNameCSV = _fileName;
    outputFileNameCSV.append(".csv");
    Log->WriteLog("Creating CSV file '%s'... ", outputFileNameCSV.c_str());

    FILE *output = fopen(outputFileNameCSV.c_str(), "w");
    if (!output)
    {
        Log->WriteLogNoTime("FAILED: Unable to create file.");
        Log->WriteLog("\n");
        return false;
    }
/*
    for (unsigned int currentField = 0; currentField < _totalFields; currentField++)
    {
        switch (_fieldTypes[currentField])
        {
            case type_FLOAT:  fprintf(output, "float"); break;
            case type_BOOL:   fprintf(output, "bool"); break;
            case type_BYTE:   fprintf(output, "byte"); break;
            case type_UBYTE:  fprintf(output, "ubyte"); break;
            case type_STRING: fprintf(output, "string"); break;
            case type_INT:    fprintf(output, "int"); break;
            case type_UINT:
            default:          fprintf(output, "uint"); break;
        }

        if (currentField + 1 < _totalFields)
            fprintf(output, ",");
    }
    fprintf(output, "\n");

    for (unsigned int currentRecord = 0; currentRecord < _totalRecords; currentRecord++)
    {
        for (unsigned int currentField = 0; currentField < _totalFields; currentField++)
        {
            if (_stringSize > 1 && _sFile.FormatedFieldTypes[currentField] == type_STRING)
            {
                unsigned int value = GetRecord(currentRecord).GetUInt(currentField);
                if (value)
                {
                    string outText = "\"";
                    for (unsigned int x = value; x < _stringSize; x++)
                    {
                        if (!_stringTable[x])
                            break;

                        if (_stringTable[x] == '"')
                            outText.append("\"");

                        if (_stringTable[x] == '\r')
                        {
                            outText.append("||||r||||");
                            continue;
                        }

                        if (_stringTable[x] == '\n')
                        {
                            outText.append("{{{{n}}}}");
                            continue;
                        }

                        if (_stringTable[x] == '\t')
                        {
                            outText.append("[[[[t]]]]");
                            continue;
                        }

                        outText.append(Shared->ToStr(_stringTable[x]));
                    }
                    outText.append("\"");
                    fprintf(output, "%s", outText.c_str());
                }
            }
            else if (_sFile.FormatedFieldTypes[currentField] == type_FLOAT)
                fprintf(output, "%f", GetRecord(currentRecord).GetFloat(currentField));
            else if (_sFile.FormatedFieldTypes[currentField] == type_BOOL)
                fprintf(output, "%u", GetRecord(currentRecord).GetBool(currentField));
            else if (_sFile.FormatedFieldTypes[currentField] == type_BYTE)
                fprintf(output, "%i", GetRecord(currentRecord).GetByte(currentField));
            else if (_sFile.FormatedFieldTypes[currentField] == type_UBYTE)
                fprintf(output, "%u", GetRecord(currentRecord).GetByte(currentField));
            else if (_sFile.FormatedFieldTypes[currentField] == type_INT)
                fprintf(output, "%i", GetRecord(currentRecord).GetInt(currentField));
            else if (_sFile.FormatedFieldTypes[currentField] == type_UINT)
                fprintf(output, "%u", GetRecord(currentRecord).GetUInt(currentField));

            if (currentField + 1 < _totalFields)
                fprintf(output, ",");
        }

        if (currentRecord + 1 < _totalRecords)
            fprintf(output, "\n");
    }
    */
    fclose(output);

    Log->WriteLogNoTime("DONE.\n");

    return true;
}