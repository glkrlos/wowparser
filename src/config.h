#ifndef _CONFIG_H_
#define _CONFIG_H_

#include "pch.h"
#include "shared.h"
#include "tinyxml2.h"
#include "log.h"

#ifdef _WIN32
    #include "win32/dirent.h"
#else
    #include <dirent.h>
#endif

using namespace tinyxml2;

class Config
{
    public:
        Config();
        bool LoadConfiguarionFile();
        void AddFilesToList(string, string, string, bool, string);
        unsigned int GetFormatedRecordSize(string);
        vector<enumFieldTypes> GetFormatedFieldTypes(string);
        unsigned int GetFormatedTotalFields(string structure)
        {
            return structure.empty() ? 0 : structure.size();
        }
        map<string, string> fileNames;
    private:
        bool IsValidFormat(string);
        void InsertIfFileNotExist(string file, string structure)
        {
            map<string, string>::iterator Found = fileNames.find(file);
            if (Found != fileNames.end())
                return;

            fileNames.insert(pair<string, string>(file, structure));
        }
    protected:
        XMLDocument XMLdoc;
};
#endif
