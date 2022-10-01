#include "../revision.h"
#include "pch.h"
#include "module_config_reader.h"

int main(int argc, char *arg[])
{
    Log->WriteLogAndPrint(LINE1);
    Log->WriteLogAndPrint(LINE2);
    Log->WriteLogAndPrint(LINE_NEW);
    Log->WriteLogAndPrint(LINE3);
    Log->WriteLogAndPrint(LINE4);
    Log->WriteLogAndPrint(LINE_NEW);

    Log->WriteLog("====================================LOG FILE START====================================\n");

    const auto_ptr<Config_Reader> Config(new Config_Reader);

    printf("*** Reading Configuration...\n");

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
        printf("*** Checking header and data consistency of each file added to parse it...\n");

        Log->WriteLog(LINE_NEW);
        Log->WriteLog("Checking header and data consistency of each file added to parse it...\n");

        /// Enviamos la lista de archivos
        FindFiles->CheckHeadersAndDataConsistencyOfAllFilesAdded();

        printf("*** Finished\n");
    }

    Log->WriteLog("====================================LOG FILE END====================================\n");

    printf("--Press any key to exit--");
    getch();
    return 0;
}
