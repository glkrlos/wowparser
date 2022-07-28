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
        sFindFiles->FileToFind(".", "", "", true, "db2");
        sFindFiles->FileToFind(".", "", "", true, "adb");
        sFindFiles->FileToFind(".", "", "", true, "csv");

        if (sFindFiles->ListEmpty())
            sLog->WriteLogAndPrint("No DBC, DB2 and ADB files found using recursive mode.\n");
        else
        {
            if (sFindFiles->TotalDBCFiles())
            {
                sLog->WriteLogAndPrint("Added to list '%i' DBC file%s using recursive mode.\n", sFindFiles->TotalDBCFiles(), sFindFiles->TotalDBCFiles() > 1 ? "s" : "");
                sFindFiles->PrintDBCFiles();
            }

            if (sFindFiles->TotalDB2Files())
            {
                sLog->WriteLogAndPrint("Added to list '%i' DB2 file%s using recursive mode.\n", sFindFiles->TotalDB2Files(), sFindFiles->TotalDB2Files() > 1 ? "s" : "");
                sFindFiles->PrintDB2Files();
            }

            if (sFindFiles->TotalADBFiles())
            {
                sLog->WriteLogAndPrint("Added to list '%i' ADB file%s using recursive mode.\n", sFindFiles->TotalADBFiles(), sFindFiles->TotalADBFiles() > 1 ? "s" : "");
                sFindFiles->PrintADBFiles();
            }

            if (sFindFiles->TotalWDBFiles())
            {
                sLog->WriteLogAndPrint("Added to list '%i' WDB file%s using recursive mode.\n", sFindFiles->TotalWDBFiles(), sFindFiles->TotalWDBFiles() > 1 ? "s" : "");
                sFindFiles->PrintWDBFiles();
            }

            if (sFindFiles->TotalCSVFiles())
            {
                sLog->WriteLogAndPrint("Added to list '%i' CSV file%s using recursive mode.\n", sFindFiles->TotalCSVFiles(), sFindFiles->TotalCSVFiles() > 1 ? "s" : "");
                sFindFiles->PrintCSVFiles();
            }

            if (sFindFiles->TotalUNKFiles())
            {
                sLog->WriteLogAndPrint("Added to list '%i' Unknown file%s using recursive mode.\n", sFindFiles->TotalUNKFiles(), sFindFiles->TotalUNKFiles() > 1 ? "s" : "");
                sFindFiles->PrintUNKFiles();
            }
        }
    }
    else
    {
        if (sFindFiles->ListEmpty())
            sLog->WriteLogAndPrint("Configuration file loaded, but no files found.\n");
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
