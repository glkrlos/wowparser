#include "../revision.h"
#include "pch.h"
#include "shared.h"
#include "module_config_reader.h"

void print_header()
{
    Log->WriteLogAndPrint("WoWParser Version 3.0 for %s (Revision: %s)\n", _OS, _REVISION);
    Log->WriteLogAndPrint("Hash: %s\tDate: %s\n", _HASH, _DATE);
    Log->WriteLogAndPrint("\n");
    Log->WriteLogAndPrint("Tool to Parse World of Warcraft files (DBC DB2 ADB WDB).\n");
    Log->WriteLogAndPrint("Copyright(c) 2022 Carlos Ramzuel - Tlaxcala, Mexico.\n");
    Log->WriteLogAndPrint("\n");

    Log->WriteLog("====================================LOG FILE START====================================\n");
}

void pass1_loadconfig()
{
    const auto_ptr<Config_Reader> Config(new Config_Reader);

    if (!Config->LoadConfiguarionFile())
    {
        Log->WriteLog("Trying to find files in recursive mode with the following extensions: %s %s %s %s\n", Shared->GetFileExtensionByFileType(dbcFile), Shared->GetFileExtensionByFileType(db2File), Shared->GetFileExtensionByFileType(adbFile), Shared->GetFileExtensionByFileType(csvFile));
        FindFiles->FileToFind(".", "", "", true, "dbc");
        FindFiles->FileToFind(".", "", "", true, "db2");
        FindFiles->FileToFind(".", "", "", true, "adb");
        FindFiles->FileToFind(".", "", "", true, "csv");

        if (FindFiles->ListEmpty())
            Log->WriteLogAndPrint("No %s, %s, %s or %s files found using recursive mode.\n", Shared->GetFileExtensionByFileType(dbcFile), Shared->GetFileExtensionByFileType(db2File), Shared->GetFileExtensionByFileType(adbFile), Shared->GetFileExtensionByFileType(csvFile));

        return;
    }

    if (FindFiles->ListEmpty())
        Log->WriteLogAndPrint("Configuration file loaded, but no files found.\n");

    return;
}

void pass2_printfilestolog()
{
    if (FindFiles->ListEmpty())
        return;

    printf("-----> Printing all files found in the log... ");

    FindFiles->PrintAllFileNamesByFileType();

    printf("DONE.\n");
}

void pass3_checkheadersanddataconsistency()
{
    if (FindFiles->ListEmpty())
        return;

    Log->WriteLog("\n");
    Log->WriteLogAndPrint("-----> Checking header and data consistency of each file added to parse it...\n");

    /// Enviamos la lista de archivos
    FindFiles->CheckHeadersAndDataConsistencyOfAllFilesAdded();
}

void print_end()
{
    Log->WriteLogAndPrint("-----> Finished\n");
    Log->WriteLog("====================================LOG FILE END====================================\n");
    printf("--Press any key to exit--\n");
    getch();
}

int main(int argc, char *arg[])
{
    print_header();

    pass1_loadconfig();

    pass2_printfilestolog();

    pass3_checkheadersanddataconsistency();

    print_end();

    return 0;
}
