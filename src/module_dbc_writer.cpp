#include "module_dbc_writer.h"

bool DBC_Writer::CreateDBCFile()
{
    string outputFileNameDBC = _fileName;
    outputFileNameDBC.append(".dbc");
    Log->WriteLog("Creating DBC file '%s'... ", outputFileNameDBC.c_str());

    if (_stringTexts.size() != _stringSize)
    {
        Log->WriteLogNoTime("FAILED: Mismatched comparison of strings.\n");
        return false;
    }

    FILE *output;
    fopen_s(&output, outputFileNameDBC.c_str(), "wb");
    if (!output)
    {
        Log->WriteLogNoTime("FAILED: Unable to create file.\n");
        return false;
    }

    fwrite("WDBC", 4, 1, output);
    fwrite(&_totalRecords, sizeof(_totalRecords), 1, output);
    fwrite(&_totalFields, sizeof(_totalFields), 1, output);
    fwrite(&_recordSize, sizeof(_recordSize), 1, output);
    fwrite(&_stringSize, sizeof(_stringSize), 1, output);

    auto currentFile = _savedData.begin();

    for (auto Records = currentFile->second.Record.begin(); Records != currentFile->second.Record.end(); Records++)
    {
        for (auto Fields = Records->Field.begin(); Fields != Records->Field.end(); Fields++)
        {
            if (Fields->Type == type_FLOAT)
            {
                float value = Fields->FloatValue;
                fwrite(&value, 4, 1, output);
            }
            else if (Fields->Type == type_BOOL)
            {
                int value = Fields->BoolValue;
                fwrite(&value, 4, 1, output);
            }
            else if (Fields->Type == type_BYTE)
            {
                char value = Fields->ByteValue;
                fwrite(&value, 1, 1, output);
            }
            else if (Fields->Type == type_UBYTE)
            {
                unsigned char value = Fields->UByteValue;
                fwrite(&value, 1, 1, output);
            }
            else if (Fields->Type == type_INT)
            {
                int value = Fields->IntValue;
                fwrite(&value, 4, 1, output);
            }
            else if (Fields->Type == type_UINT)
            {
                unsigned int value = Fields->UIntValue;
                fwrite(&value, 4, 1, output);
            }
            else
                /// String
            {
                unsigned int value = Fields->StringValue;
                fwrite(&value, 4, 1, output);
            }
        }
    }

    fwrite(_stringTexts.c_str(), _stringTexts.size(), 1, output);

    fclose(output);

    Log->WriteLogNoTime("DONE.\n");

    return true;
}
