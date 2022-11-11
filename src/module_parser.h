#ifndef _MODULE_PARSER_H_
#define _MODULE_PARSER_H_

#include "pch.h"
#include "shared.h"

struct structDBCHeader
{
    char header[4];
    unsigned int totalRecords;
    unsigned int totalFields;
    unsigned int recordSize;
    unsigned int stringSize;
};

enum enumErrorTypes
{
    DATA_READ_ERROR,
    ASCII_FILE,
    BINARY_FILE
};

class module_parser
{
    public:
        module_parser(structFile &sFile)
        {
            _sFile = sFile;
        }
        ~module_parser()
        {
            if (_fileData)
            {
                delete _fileData;
                _fileData = NULL;
            }

            if (_inputFile) fclose(_inputFile);
        }
        bool Load();
    private:
        const char *GetFileName() { return _sFile.FileName.c_str(); }
        enumErrorTypes FileIsASCII()
        {
            if (!_fileData)
                return DATA_READ_ERROR;

            for (auto x = 0; x < _fileSize; x++)
            {
                if (static_cast<char>(_fileData[x]) == '\n' || static_cast<char>(_fileData[x]) == '\r')
                    continue;

                if (!isprint(static_cast<char>(_fileData[x])))
                    return BINARY_FILE;
            }

            return ASCII_FILE;
        }
    protected:
        structFile _sFile;
        FILE *_inputFile;
        long _fileSize = 0;
        unsigned char *_fileData = NULL;
};
#endif