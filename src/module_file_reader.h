#ifndef _MODULE_FILE_READER_H_
#define _MODULE_FILE_READER_H_

#include "pch.h"
#include "shared.h"

class File_Reader
{
    public:
        File_Reader()
        {
            _inputFile = NULL;
            _fileSize = 0;
            _wholeFileData = NULL;
            _isASCIIFile = true;
        }
        ~File_Reader()
        {
            _inputFile = NULL;
            _fileSize = 0;

            if (_wholeFileData)
            {
                delete _wholeFileData;
                _wholeFileData = NULL;
            }

            _isASCIIFile = true;
        }
        File_Reader *Export()
        {
            return this;
        }
        bool Load();
    private:
        bool NullPoniterToData() { return !_wholeFileData; }
        void CheckIfIsASCIIorBinaryFile()
        {
            _isASCIIFile = true;

            for (unsigned long x = 0; x < _fileSize; x++)
            {
                if (static_cast<char>(_wholeFileData[x]) == '\n' || static_cast<char>(_wholeFileData[x]) == '\r')
                    continue;

                if (!isprint(static_cast<char>(_wholeFileData[x])))
                {
                    _isASCIIFile = false;
                    break;
                }
            }
        }
    protected:
        FILE *_inputFile;
        unsigned long _fileSize;
        structXMLFileInfo _XMLFileInfo;
        unsigned char *_wholeFileData;

        bool _isASCIIFile;
};

#endif
