#include "../revision.h"
#include "pch.h"
#include "module_config_reader.h"
#include "module_binary_reader.h"

int main(int argc, char *arg[])
{
    sLog->WriteLog(LINE1);
    sLog->WriteLog(LINE2);
    sLog->WriteLog(LINE_NEW);
    sLog->WriteLog(LINE3);
    sLog->WriteLog(LINE4);
    sLog->WriteLog(LINE_NEW);

    sLog->WriteLog("====================================LOG FILE START====================================\n");

    const auto_ptr<Config_Reader> Config(new Config_Reader);

    if (!Config->LoadConfiguarionFile())
    {
        sLog->WriteLog("Trying to find files in recursive mode with the following extensions: %s %s %s %s\n", sFindFiles->GetFileExtensionByFileType(dbcFile), sFindFiles->GetFileExtensionByFileType(db2File), sFindFiles->GetFileExtensionByFileType(adbFile), sFindFiles->GetFileExtensionByFileType(csvFile));
        sFindFiles->FileToFind(".", "", "", true, "dbc");
        sFindFiles->FileToFind(".", "", "", true, "db2");
        sFindFiles->FileToFind(".", "", "", true, "adb");
        sFindFiles->FileToFind(".", "", "", true, "csv");

        if (sFindFiles->ListEmpty())
            sLog->WriteLogAndPrint("No %s, %s, %s or %s files found using recursive mode.\n", sFindFiles->GetFileExtensionByFileType(dbcFile), sFindFiles->GetFileExtensionByFileType(db2File), sFindFiles->GetFileExtensionByFileType(adbFile), sFindFiles->GetFileExtensionByFileType(csvFile));
        else
            sFindFiles->PrintAllFileNamesByFileType();
    }
    else
    {
        if (sFindFiles->ListEmpty())
            sLog->WriteLogAndPrint("Configuration file loaded, but no files found.\n");
        else
            sFindFiles->PrintAllFileNamesByFileType();
    }

    // Enviamos la infor
    //const auto_ptr<BinaryReader> BR(new BinaryReader());


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
    sLog->WriteLog("====================================LOG FILE END====================================\n");

    printf(LINE1);
    printf(LINE2);
    printf(LINE_NEW);
    printf(LINE3);
    printf(LINE4);
    printf(LINE_NEW);

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
