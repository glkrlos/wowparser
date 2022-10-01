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

struct structFile
{
    enumFileType Type;
    string Structure;
    bool isRecursivelySearched;
    bool isSearchedByExtension;
    unsigned int XMLFileID;
};

class cFindFiles
{
    public:
        void CheckHeadersAndDataConsistencyOfAllFilesAdded();
        cFindFiles();
        const char *GetFileExtensionByFileType(enumFileType eFT);
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
        //vector<unsigned int> countFiles;
        map<string, enumFileType> _fileExtensions;
};

#define FindFiles CSingleton<cFindFiles>::Instance()
#endif
