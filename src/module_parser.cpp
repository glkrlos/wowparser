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
        return false;
    }

    if (_fileSize < 5)
    {
        Log->WriteLogNoTime("FAILED: File size is too small. Are you sure is a %s file ?");
        return false;
    }

    rewind(_inputFile);

    _fileData = new unsigned char[_fileSize];
    if (fread(_fileData, _fileSize, 1, _inputFile) != 1)
    {
        Log->WriteLogNoTime("FAILED: Unable to read file.\n");
        return false;
    }

    if (FileIsASCII() == DATA_READ_ERROR)
    {
        Log->WriteLogNoTime("FATAL: DATA_READ_ERROR: null Pointer to file data. Report this to fix it.\n");
        return false;
    }

    if (FileIsASCII() == ASCII_FILE)
        ; // Pasar a funcion csv_reader, cerrando el archivo aqui
    else
        GetFileTypeByHeader();

    Log->WriteLogNoTime("DONE.\n");

    return true;
}

enumFileType module_parser::GetFileTypeByHeader()
{
    string header = "";
    header.append(Shared->ToStr(GetChar()));
    header.append(Shared->ToStr(GetChar()));
    header.append(Shared->ToStr(GetChar()));
    header.append(Shared->ToStr(GetChar()));

    printf("'%s'\n", header.c_str());
    _getch();

    return unkFile;
}