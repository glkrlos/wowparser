#include "../revision.h"
#include "pch.h"
#include "shared.h"
#include "config_reader.h"

#include "dbc_reader.h"

int main(int argc, char *arg[])
{
    FILE *logFile;
    logFile = fopen(WoWParserLogOutPut, "w");
    if (logFile)
        fclose(logFile);

    WriteLog("WoWParser Version 3.0 BETA for %s   (Revision: %s)\n", _OS, _REVISION);
    WriteLog("Hash: %s\tDate: %s\n", _HASH, _DATE);
    WriteLog("Tool to Parse World of Warcraft files (DBC DB2 ADB WDB).\n");
    WriteLog("Copyright(c) 2022 Carlos Ramzuel - Tlaxcala, Mexico.\n");
    WriteLog("\n");
    WriteLog("======================LOG FILE START======================\n");
    
    Config_Reader Cfg;
    if (!Cfg.LoadConfiguarionFile())
    {
        Cfg.AddFilesToList(".", "", "", true, "dbc");
        unsigned int dbcFilesLoaded = Cfg.fileNames.size();
        Cfg.AddFilesToList(".", "", "", true, "db2");
        unsigned int db2FilesLoaded = Cfg.fileNames.size() > dbcFilesLoaded ? Cfg.fileNames.size() - dbcFilesLoaded : 0;
        Cfg.AddFilesToList(".", "", "", true, "adb");
        unsigned int adbFilesLoaded = Cfg.fileNames.size() > (dbcFilesLoaded + db2FilesLoaded) ? Cfg.fileNames.size() - dbcFilesLoaded - db2FilesLoaded : 0;

        if (Cfg.fileNames.empty())
            WriteLogAndPrint("No DBC, DB2 and ADB files found using recursive mode.\n");

        map<string, string>::iterator FileName = Cfg.fileNames.begin();
        if (dbcFilesLoaded)
        {
            WriteLogAndPrint("Automatic added to list '%i' DBC file%s using recursive mode.\n", dbcFilesLoaded, dbcFilesLoaded > 1 ? "s" : "");
            for (unsigned int x = 0; x < dbcFilesLoaded; x++)
            {
                WriteLog("File: %s\n", FileName->first.c_str());
                FileName++;
            }
        }

        if (db2FilesLoaded)
        {
            WriteLogAndPrint("Automatic added to list '%i' DB2 file%s using recursive mode.\n", db2FilesLoaded, db2FilesLoaded > 1 ? "s" : "");
            for (unsigned int x = 0; x < dbcFilesLoaded; x++)
            {
                WriteLog("File: %s\n", FileName->first.c_str());
                FileName++;
            }
        }

        if (adbFilesLoaded)
        {
            WriteLogAndPrint("Automatic added to list '%i' ADB file%s using recursive mode.\n", adbFilesLoaded, adbFilesLoaded > 1 ? "s" : "");
            for (unsigned int x = 0; x < dbcFilesLoaded; x++)
            {
                WriteLog("File: %s\n", FileName->first.c_str());
                FileName++;
            }
        }
    }
    else
    {
        if (Cfg.fileNames.empty())
            WriteLogAndPrint("Configuration file loaded, but no files found.\n");
    }

    for (map<string, string>::iterator FileName = Cfg.fileNames.begin(); FileName != Cfg.fileNames.end(); FileName++)
    {
        vector<enumFieldTypes> FormatedFieldTypes = Cfg.GetFormatedFieldTypes(FileName->second);
        unsigned int FormatedTotalFields = Cfg.GetFormatedTotalFields(FileName->second);
        unsigned int FormatedRecordSize = Cfg.GetFormatedRecordSize(FileName->second);

        DBCReader dbcReader(FileName->first.c_str(), FormatedFieldTypes, FormatedTotalFields, FormatedRecordSize);
        dbcReader.Load();
    }

    WriteLog("=======================LOG FILE END=======================\n");

    printf("WoWParser Version 3.0 BETA for %s   (Revision: %s)\n", _OS, _REVISION);
    printf("Hash: %s\tDate: %s\n", _HASH, _DATE);
    printf("\nTool to Parse World of Warcraft files (DBC DB2 ADB WDB).\n");
    printf("Copyright(c) 2022 Carlos Ramzuel - Tlaxcala, Mexico.\n");

    getch();
    return 0;
}
