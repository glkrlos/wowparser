#ifndef _FINDFILES_H_
#define _FINDFILES_H_

#include "pch.h"
#include "shared.h"
#include <algorithm>
#include "module_parser.h"

#ifdef _WIN32
    #include "win32/dirent.h"
#else
    #include <dirent.h>
#endif

#include "ProgressBar.h"

class cFindFiles
{
    public:
        void CheckHeadersAndDataConsistencyOfAllFilesAdded();
        cFindFiles();
        void FileToFind(string directory, string filename, string structure, bool recursive, string fileExt, unsigned int xmlFileID = 0);
        void PrintAllFileNamesByFileType();
        bool ListEmpty();
    private:
        enumFileType GetFileTypeByExtension(string FileName);
        bool HaveExtension(string fileName);
        string GetFileExtension(string fileName);
        void AddFileToListIfNotExist(string fileName, structFile File);
    protected:
        map<string, structFile> fileNames;
};

#define FindFiles CSingleton<cFindFiles>::Instance()
#endif
