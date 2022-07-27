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
    private:
    protected:
        XMLDocument XMLdoc;
};
#endif
