#include "../revision.h"
#include "pch.h"
#include "config_xml.h"
//#include "shared.h"
//#include "findfiles.h"
//#include "module_binary_reader.h"

#define Cfg CSingleton<Config>::Instance()

int main(int argc, char *arg[])
{
    sLog->WriteLog("WoWParser Version 3.0 (Stable) for %s (Revision: %s)\n", _OS, _REVISION);
    sLog->WriteLog("Hash: %s\tDate: %s\n", _HASH, _DATE);
    sLog->WriteLog("Tool to Parse World of Warcraft files (DBC DB2 ADB WDB).\n");
    sLog->WriteLog("Copyright(c) 2022 Carlos Ramzuel - Tlaxcala, Mexico.\n");
    sLog->WriteLog("\n");
    sLog->WriteLog("======================LOG FILE START======================\n");

    if (!Cfg->LoadConfiguarionFile())
    {
        sFindFiles->FileToFind(".", "", "", true, "dbc");
        sFindFiles->FileToFind(".", "", "", true, "db2");
        sFindFiles->FileToFind(".", "", "", true, "adb");
        sFindFiles->FileToFind(".", "", "", true, "csv");

        if (sFindFiles->ListEmpty())
            sLog->WriteLogAndPrint("No DBC, DB2 and ADB files found using recursive mode.\n");
        else
            sFindFiles->PrintAllFileNamesByFileType();
    }
    else
    {
        if (sFindFiles->ListEmpty())
            sLog->WriteLogAndPrint("Configuration file loaded, but no files found.\n");
        else
        {
        }
    }

/*
    for (auto FileName = Cfg->fileNames.begin(); FileName != Cfg->fileNames.end(); FileName++)
    {
        vector<enumFieldTypes> FormatedFieldTypes = Cfg->GetFormatedFieldTypes(FileName->second);
        unsigned int FormatedTotalFields = Cfg->GetFormatedTotalFields(FileName->second);
        unsigned int FormatedRecordSize = Cfg->GetFormatedRecordSize(FileName->second);

        //DBCReader dbcReader(FileName->first.c_str(), FormatedFieldTypes, FormatedTotalFields, FormatedRecordSize);
        //dbcReader.Load();
    }
*/
    sLog->WriteLog("=======================LOG FILE END=======================\n");

    printf("WoWParser Version 3.0 (Stable) for %s (Revision: %s)\n", _OS, _REVISION);
    printf("Hash: %s\tDate: %s\n", _HASH, _DATE);
    printf("\n");
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
