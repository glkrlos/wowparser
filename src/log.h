#ifdef __WIN32__
    #include <time.h>
#endif

#define WoWParserLogOutPut "wowparser3.log"

inline void CreateEmptyLogFile()
{
    FILE *logFile;
    logFile = fopen(WoWParserLogOutPut, "w");
    if (logFile)
        fclose(logFile);
}

inline void WriteLog(const char* args, ...)
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

inline void WriteLogNoTime(const char* args, ...)
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

inline void WriteLogAndPrint(const char* args, ...)
{
    va_list ap;
    va_start(ap, args);
    char outstr[4096];
    vsnprintf(outstr, 4096, args, ap);
    va_end(ap);

    printf("%s", outstr);

    WriteLog(outstr);
}
