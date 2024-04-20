#ifndef MODULE_CONFIG_READER_H
#define MODULE_CONFIG_READER_H

#include "pch.h"
#include "shared.h"
#include "../lib/tinyxml2/tinyxml2.h"
#include "findfiles.h"

#ifdef _WIN32
    #include "win32/dirent.h"
#else
    #include <dirent.h>
#endif

#define PARSERFILENAME "wowparser4.xml"

using namespace tinyxml2;

class Config_Reader
{
    public:
        bool LoadConfiguarionFile();
    private:
        static bool IsValidFormat(const string& structure)
        {
            for (char x : structure)
            {
                switch (x)
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
