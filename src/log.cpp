#include "log.h"

Log::Log()
{
    FILE *logFile;
    logFile = fopen(WoWParserLogOutPut, "w");
    if (logFile)
        fclose(logFile);
}

void Log::WriteLog(const char* args, ...)
{
    FILE *logFile = fopen(WoWParserLogOutPut, "a");
    if (!logFile)
        return;

    va_list ap;
    va_start(ap, args);
    char outstr[4096];
    vsnprintf(outstr, 4096, args, ap);
    va_end(ap);

    time_t rawtime;
    struct tm * timeinfo;
    char buffer [80];

    time ( &rawtime );
    timeinfo = localtime ( &rawtime );

    strftime (buffer,80,"%Y-%m-%d %H:%M:%S",timeinfo);

    fprintf(logFile, "%s %s", buffer, outstr);
    fclose(logFile);
}

void Log::WriteLogNoTime(const char* args, ...)
{
    FILE *logFile = fopen(WoWParserLogOutPut, "a");
    if (!logFile)
        return;

    va_list ap;
    va_start(ap, args);
    char outstr[4096];
    vsnprintf(outstr, 4096, args, ap);
    va_end(ap);

    fprintf(logFile, "%s", outstr);
    fclose(logFile);
}

void Log::WriteLogAndPrint(const char* args, ...)
{
    va_list ap;
    va_start(ap, args);
    char outstr[4096];
    vsnprintf(outstr, 4096, args, ap);
    va_end(ap);

    printf("%s", outstr);

    WriteLog(outstr);
}
