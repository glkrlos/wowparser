#ifndef _LOG_H_
#define _LOG_H_

#include "pch.h"
#include "shared.h"

#include <cstdarg>
#ifdef _WIN32
    #include <ctime>
#endif

#define WoWParserLogOutPut "wowparser3.log"

class cLog
{
    public:
        cLog();
        static cLog* Instance();
        static void WriteLog(const char* args, ...);
        static void WriteLogNoTime(const char* args, ...);
        static void WriteLogNoTimeAndPrint(const char* args, ...);
        static void WriteLogAndPrint(const char* args, ...);
};

#define Log cLog::Instance()
#endif
