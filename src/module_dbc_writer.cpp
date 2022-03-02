#include "module_dbc_writer.h"

DBC_Writer::DBC_Writer(FileData *_fileData)
{
    _file = _fileData;
}

DBC_Writer::~DBC_Writer()
{
    _file = NULL;
}

void DBC_Writer::CreateDBC()
{
    string outFileName = _file->GetFileName();
    outFileName += ".dbc";

    if (_file->GetStringSize() != _file->GetStringTextsSize())
    {
        printf("ERROR: '%s':\n\tThis should never happen 'GetStringSize() != GetStringTextsSize()'\n\tUnable to create DBC file '%s'.", _file->GetFileName(), outFileName.c_str());
        return;
    }

    FILE *output;
    fopen_s(&output, outFileName.c_str(), "wb");
    if (!output)
    {
        printf("ERROR: '%s': Can't create DBC file.\n", outFileName.c_str());
        return;
    }

    structDBCHeader DBCHeader;
    DBCHeader.headerName[0] = 'W';
    DBCHeader.headerName[1] = 'D';
    DBCHeader.headerName[2] = 'B';
    DBCHeader.headerName[3] = 'C';
    DBCHeader.totalRecords = _file->GetTotalRecords();
    DBCHeader.totalFields = _file->GetTotalFields();
    DBCHeader.recordSize = _file->GetRecordSize();
    DBCHeader.stringSize = _file->GetStringSize();

    fwrite(&DBCHeader, sizeof(structDBCHeader), 1, output);

    while (_file->NextRecord())
    {
        Records currentRecord(_file->GetRecord());
        while (currentRecord.NextField())
        {
            Fields currentField(currentRecord.GetField());
            if (currentField.GetType() == type_FLOAT)
            {
                float value = float(currentField.GetFloat());
                fwrite(&value, 4, 1, output);
            }
            else if (currentField.GetType() == type_BYTE || currentField.GetType() == type_UBYTE)
            {
                int value = currentField.GetInt();
                fwrite(&value, 1, 1, output);
            }
            else
            {
                int value = currentField.GetInt();
                fwrite(&value, 4, 1, output);
            }
        }
    }

    fwrite(_file->GetStringTexts(), _file->GetStringSize(), 1, output);

    fclose(output);

    printf("DBC file created '%s'.\n", outFileName.c_str());
}