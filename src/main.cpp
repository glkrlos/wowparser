#include "../revision.h"
#include "pch.h"
#include "module_config_reader.h"

void print_header()
{
    Log->WriteLogAndPrint(LINE1);
    Log->WriteLogAndPrint(LINE2);
    Log->WriteLogAndPrint(LINE_NEW);
    Log->WriteLogAndPrint(LINE3);
    Log->WriteLogAndPrint(LINE4);
    Log->WriteLogAndPrint(LINE_NEW);
    Log->WriteLogAndPrint(LINE_NEW);

    Log->WriteLog("====================================LOG FILE START====================================\n");
}

void pass1_loadconfig()
{
    printf("*** Reading Configuration...\n");

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

    printf("*** Printing all files found in the log...\n");

    FindFiles->PrintAllFileNamesByFileType();
}

void pass3_checkheadersanddataconsistency()
{
    if (FindFiles->ListEmpty())
        return;

    printf("*** Checking header and data consistency of each file added to parse it...\n");

    Log->WriteLog(LINE_NEW);
    Log->WriteLog("Checking header and data consistency of each file added to parse it...\n");

    /// Enviamos la lista de archivos
    FindFiles->CheckHeadersAndDataConsistencyOfAllFilesAdded();
}

void print_end()
{
    printf("*** Finished\n");
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
