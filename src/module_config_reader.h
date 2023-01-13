#ifndef _MODULE_CONFIG_READER_H_
#define _MODULE_CONFIG_READER_H_

#include "pch.h"
#include "shared.h"
#include "tinyxml2.h"
#include "findfiles.h"

#ifdef _WIN32
    #include "win32/dirent.h"
#else
    #include <dirent.h>
#endif

using namespace tinyxml2;

class Config_Reader
{
    public:
        Config_Reader();
        bool LoadConfiguarionFile();
    private:
        bool IsValidFormat(string structure)
        {
            for (unsigned int x = 0; x < structure.size(); x++)
            {
                switch (structure[x])
                {
                    case 'X':   // unk byte
                    case 'b':   // byte
                    case 's':   // string
                    case 'f':   // float
                    case 'd':   // int
                    case 'n':   // int
                    case 'x':   // unk int
                    case 'i':   // int
                    case 'u':   // unsigned int
                        break;
                    default:
                        return false;
                }
            }

            return true;
        }
    protected:
        XMLDocument XMLdoc;
};
#endif
