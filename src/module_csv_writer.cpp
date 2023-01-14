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

    for (auto currentField = _fieldTypes.begin(); currentField != _fieldTypes.end(); currentField++)
    {
        switch (*currentField)
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

        if (currentField + 1 != _fieldTypes.end())
            fprintf(output, ",");
    }
    fprintf(output, "\n");

    auto currentFile = _savedData.begin();
    for (auto Records = currentFile->second.Record.begin(); Records != currentFile->second.Record.end(); Records++)
    {
        for (auto Fields = Records->Field.begin(); Fields != Records->Field.end(); Fields++)
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

                        if (_stringTexts[x] == '\r')
                        {
                            outText.append("||||r||||");
                            continue;
                        }

                        if (_stringTexts[x] == '\n')
                        {
                            outText.append("{{{{n}}}}");
                            continue;
                        }

                        if (_stringTexts[x] == '\t')
                        {
                            outText.append("[[[[t]]]]");
                            continue;
                        }

                        outText.append(Shared->ToStr(_stringTexts[x]));
                    }
                    outText.append("\"");
                    fprintf(output, "%s", outText.c_str());
                }
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

            if (Fields + 1 != Records->Field.end())
                fprintf(output, ",");
        }

        if (Records + 1 != currentFile->second.Record.end())
            fprintf(output, "\n");
    }

    fclose(output);

    Log->WriteLogNoTime("DONE.\n");

    return true;
}