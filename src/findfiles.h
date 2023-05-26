#ifndef _FINDFILES_H_
#define _FINDFILES_H_

#include "pch.h"
#include "shared.h"
#include "module_parser.h"

#ifdef _WIN32
    #include "win32/dirent.h"
#else
    #include <dirent.h>
#endif

class cFindFiles
{
    public:
        static cFindFiles* Instance();
        map<string, structXMLFileInfo> XMLFileInfo() { return fileNames; }
        cFindFiles();
        void FileToFind(string directory, string filename, string structure, bool recursive, string fileExt, outputFormat outFormats, unsigned int xmlFileID = 0);
        void PrintAllFileNamesByFileType();
        bool ListEmpty();
    private:
        enumFileType GetFileTypeByExtension(string FileName);
        bool HaveExtension(string fileName);
        string GetFileExtension(string fileName);
        void AddFileToListIfNotExist(string fileName, structXMLFileInfo File);
        unsigned int GetFormatedTotalFields(string structure)
        {
            return structure.empty() ? 0 : structure.size();
        }
        unsigned int GetFormatedRecordSize(string structure)
        {
            unsigned int RecordSize = 0;

            for (unsigned int x = 0; x < structure.size(); x++)
            {
                switch (structure[x])
                {
                    case 'X':   // unk byte
                    case 'b':   // byte
                        RecordSize += 1;
                        break;
                    default:
                        RecordSize += 4;
                        break;
                }
            }

            return RecordSize;
        }
        vector<enumFieldTypes> GetFormatedFieldTypes(string structure)
        {
            vector<enumFieldTypes> fieldTypes;
            for (unsigned int x = 0; x < structure.size(); x++)
            {
                switch (structure[x])
                {
                    case 'X':   // unk byte
                    case 'b':   // byte
                        fieldTypes.push_back(type_BYTE);
                        continue;
                    case 's':   // string
                        fieldTypes.push_back(type_STRING);
                        continue;
                    case 'f':   // float
                        fieldTypes.push_back(type_FLOAT);
                        continue;
                    case 'd':   // int
                    case 'n':   // int
                    case 'x':   // unk int
                    case 'i':   // int
                        fieldTypes.push_back(type_INT);
                        continue;
                    case 'u':   // unsigned int
                        fieldTypes.push_back(type_UINT);
                        continue;
                    default:
                        fieldTypes.push_back(type_NONE);
                        continue;
                }
            }

            return fieldTypes;
        }

    protected:
        map<string, structXMLFileInfo> fileNames;
};

#define FindFiles cFindFiles::Instance()
#endif
