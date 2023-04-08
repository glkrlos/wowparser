#include "module_file_reader.h"

bool File_Reader::Load()
{
    _inputFile = fopen(_XMLFileInfo.FileName.c_str(), "rb");
    if (!_inputFile)
    {
        Log->WriteLog("ERROR: Can't open file '%s'.\n", _XMLFileInfo.FileName.c_str());
        return false;
    }

    Log->WriteLog("Reading file '%s'... ", _XMLFileInfo.FileName.c_str());

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

    /// Obtenemos el tipo de archivo ASCII o Binario
    CheckIfIsASCIIorBinaryFile();

    /// Para los archivos csv, si fuera solo la palabra "int" serian 3 bytes al menos
    /// Para los archivos binarios, debe tener al menos 20 bytes de datos al inicio
    if (((_XMLFileInfo.Type == csvFile || _isASCIIFile) && _fileSize < 3) || (!_isASCIIFile && _fileSize < 20))
    {
        Log->WriteLogNoTime("FAILED: File size is too small. Are you sure is a '%s' file?\n", Shared->GetFileExtensionByFileType(_XMLFileInfo.Type));
        Log->WriteLog("\n");
        return false;
    }

    if (_XMLFileInfo.Type == csvFile || _isASCIIFile)
    {
        Log->WriteLogNoTime("DONE.\n");
        Log->WriteLog("Parsing file... ");
    }

    Log->WriteLogNoTime("DONE.\n");
    return true;
}