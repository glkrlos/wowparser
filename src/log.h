#ifndef _LOG_H_
#define _LOG_H_

#include "pch.h"
#include "shared.h"

#include <memory>
#include <stdarg.h>
#ifdef _WIN32
    #include <time.h>
#endif

#define WoWParserLogOutPut "wowparser3.log"

class cLog
{
    public:
        cLog();
        void WriteLog(const char* args, ...);
        void WriteLogNoTime(const char* args, ...);
        void WriteLogAndPrint(const char* args, ...);
};

#define Log CSingleton<cLog>::Instance()
#endif
