#ifndef LOG_H
#define LOG_H

#include "pch.h"
#include "shared.h"

#include <cstdarg>
#ifdef _WIN32
    #include <ctime>
#endif

#define WoWParserLogOutPut "wowparser4.log"

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
