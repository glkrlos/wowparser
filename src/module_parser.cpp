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
    else if (FileIsASCII() == ASCII_FILE)
        Log->WriteLogNoTime("");
    else
        Log->WriteLogNoTime("");

    Log->WriteLogNoTime("DONE.\n");

    return true;
}