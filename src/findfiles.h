#ifndef _FINDFILES_H_
#define _FINDFILES_H_

#include "pch.h"
#include "shared.h"

#ifdef _WIN32
    #include "win32/dirent.h"
#else
    #include <dirent.h>
#endif

class FindFiles
{
    public:
        void FileToFind(string, string, string, bool, string);
    private:
        void AddFileToListIfNotExist(string file, string structure)
        {
            map<string, string>::iterator Found = fileNames.find(file);
            if (Found != fileNames.end())
                return;

            fileNames.insert(pair<string, string>(file, structure));
        }
    protected:
        map<string, string> fileNames;
};

#define sFindFiles CSingleton<FindFiles>::Instance()
#endif