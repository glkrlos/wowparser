#include "../revision.h"
#include "pch.h"
#include "config_xml.h"
//#include "shared.h"
//#include "findfiles.h"
//#include "module_binary_reader.h"

#define Cfg CSingleton<Config>::Instance()

int main(int argc, char *arg[])
{
    sLog->WriteLog("WoWParser Version 3.0 BETA for %s   (Revision: %s)\n", _OS, _REVISION);
    sLog->WriteLog("Hash: %s\tDate: %s\n", _HASH, _DATE);
    sLog->WriteLog("Tool to Parse World of Warcraft files (DBC DB2 ADB WDB).\n");
    sLog->WriteLog("Copyright(c) 2022 Carlos Ramzuel - Tlaxcala, Mexico.\n");
    sLog->WriteLog("\n");
    sLog->WriteLog("======================LOG FILE START======================\n");

    if (!Cfg->LoadConfiguarionFile())
    {
        sFindFiles->FileToFind(".", "", "", true, "dbc");
        unsigned int dbcFilesLoaded = Cfg->fileNames.size();
        sFindFiles->FileToFind(".", "", "", true, "db2");
        unsigned int db2FilesLoaded = Cfg->fileNames.size() > dbcFilesLoaded ? Cfg->fileNames.size() - dbcFilesLoaded : 0;
        sFindFiles->FileToFind(".", "", "", true, "adb");
        unsigned int adbFilesLoaded = Cfg->fileNames.size() > (dbcFilesLoaded + db2FilesLoaded) ? Cfg->fileNames.size() - dbcFilesLoaded - db2FilesLoaded : 0;

        if (Cfg->fileNames.empty())
            sLog->WriteLogAndPrint("No DBC, DB2 and ADB files found using recursive mode.\n");

        map<string, string>::iterator FileName = Cfg->fileNames.begin();
        if (dbcFilesLoaded)
        {
            sLog->WriteLogAndPrint("Automatic added to list '%i' DBC file%s using recursive mode.\n", dbcFilesLoaded, dbcFilesLoaded > 1 ? "s" : "");
            for (unsigned int x = 0; x < dbcFilesLoaded; x++)
            {
                sLog->WriteLog("File: %s\n", FileName->first.c_str());
                FileName++;
            }
        }

        if (db2FilesLoaded)
        {
            sLog->WriteLogAndPrint("Automatic added to list '%i' DB2 file%s using recursive mode.\n", db2FilesLoaded, db2FilesLoaded > 1 ? "s" : "");
            for (unsigned int x = 0; x < dbcFilesLoaded; x++)
            {
                sLog->WriteLog("File: %s\n", FileName->first.c_str());
                FileName++;
            }
        }

        if (adbFilesLoaded)
        {
            sLog->WriteLogAndPrint("Automatic added to list '%i' ADB file%s using recursive mode.\n", adbFilesLoaded, adbFilesLoaded > 1 ? "s" : "");
            for (unsigned int x = 0; x < dbcFilesLoaded; x++)
            {
                sLog->WriteLog("File: %s\n", FileName->first.c_str());
                FileName++;
            }
        }
    }
    else
    {
        if (Cfg->fileNames.empty())
            sLog->WriteLogAndPrint("Configuration file loaded, but no files found.\n");
    }

    for (auto FileName = Cfg->fileNames.begin(); FileName != Cfg->fileNames.end(); FileName++)
    {
        vector<enumFieldTypes> FormatedFieldTypes = Cfg->GetFormatedFieldTypes(FileName->second);
        unsigned int FormatedTotalFields = Cfg->GetFormatedTotalFields(FileName->second);
        unsigned int FormatedRecordSize = Cfg->GetFormatedRecordSize(FileName->second);

        //DBCReader dbcReader(FileName->first.c_str(), FormatedFieldTypes, FormatedTotalFields, FormatedRecordSize);
        //dbcReader.Load();
    }

    sLog->WriteLog("=======================LOG FILE END=======================\n");

    printf("WoWParser Version 3.0 BETA for %s   (Revision: %s)\n", _OS, _REVISION);
    printf("Hash: %s\tDate: %s\n", _HASH, _DATE);
    printf("\nTool to Parse World of Warcraft files (DBC DB2 ADB WDB).\n");
    printf("Copyright(c) 2022 Carlos Ramzuel - Tlaxcala, Mexico.\n");
    /*
    sFindFiles->FileToFind(".", "", "", true, "dbc");
    vector<enumFieldTypes> FT;
    FT.clear();
    BinaryReader aaa("Achievement.dbc", FT, 0, 0);
    aaa.Load();
*/
    getch();
    return 0;
}
