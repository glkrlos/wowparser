#ifndef _CONFIG_XML_H_
#define _CONFIG_XML_H_

#include "pch.h"
#include "shared.h"
#include "tinyxml2.h"
#include "findfiles.h"
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
        unsigned int GetFormatedRecordSize(string);
        vector<enumFieldTypes> GetFormatedFieldTypes(string);
        unsigned int GetFormatedTotalFields(string structure)
        {
            return structure.empty() ? 0 : structure.size();
        }
    private:
        bool IsValidFormat(string);
    protected:
        XMLDocument XMLdoc;
};
#endif
