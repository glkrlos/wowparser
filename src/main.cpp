#include "../revision.h"
#include "pch.h"
#include "module_config_reader.h"

int main(int argc, char *arg[])
{
    Log->WriteLog(LINE1);
    Log->WriteLog(LINE2);
    Log->WriteLog(LINE_NEW);
    Log->WriteLog(LINE3);
    Log->WriteLog(LINE4);
    Log->WriteLog(LINE_NEW);

    Log->WriteLog("====================================LOG FILE START====================================\n");

    const auto_ptr<Config_Reader> Config(new Config_Reader);

    if (!Config->LoadConfiguarionFile())
    {
        Log->WriteLog("Trying to find files in recursive mode with the following extensions: %s %s %s %s\n", FindFiles->GetFileExtensionByFileType(dbcFile), FindFiles->GetFileExtensionByFileType(db2File), FindFiles->GetFileExtensionByFileType(adbFile), FindFiles->GetFileExtensionByFileType(csvFile));
        FindFiles->FileToFind(".", "", "", true, "dbc");
        FindFiles->FileToFind(".", "", "", true, "db2");
        FindFiles->FileToFind(".", "", "", true, "adb");
        FindFiles->FileToFind(".", "", "", true, "csv");

        if (FindFiles->ListEmpty())
            Log->WriteLogAndPrint("No %s, %s, %s or %s files found using recursive mode.\n", FindFiles->GetFileExtensionByFileType(dbcFile), FindFiles->GetFileExtensionByFileType(db2File), FindFiles->GetFileExtensionByFileType(adbFile), FindFiles->GetFileExtensionByFileType(csvFile));
        else
            FindFiles->PrintAllFileNamesByFileType();
    }
    else
    {
        if (FindFiles->ListEmpty())
            Log->WriteLogAndPrint("Configuration file loaded, but no files found.\n");
        else
            FindFiles->PrintAllFileNamesByFileType();
    }

    if (!FindFiles->ListEmpty())
    {
        Log->WriteLog(LINE_NEW);
        Log->WriteLog("Checking header and data consistency of each file added to parse it...\n");

        /// Enviamos la lista de archivos
        FindFiles->CheckHeadersAndDataConsistencyOfAllFilesAdded();
    }

/*

*/
    Log->WriteLog("====================================LOG FILE END====================================\n");

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
