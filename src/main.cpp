#include "../revision.h"
#include "pch.h"
#include "shared.h"
#include "findfiles.h"
#include "module_binary_reader.h"
#include "config_xml.h"

int main(int argc, char *arg[])
{
    FindFiles Files;
    Files.FileToFind(".", "", "", true, "dbc");
    vector<enumFieldTypes> FT;
    FT.clear();
    BinaryReader aaa("Achievement.dbc", FT, 0, 0);
    aaa.Load();

    printf("\n\n");
    printf("WoWParser Version 3.0 for %s (Revision: %s)\n", _OS, _REVISION);
    printf("Hash: %s Date: %s\n", _HASH, _DATE);
    printf("\nTool to Parse World of Warcraft files (DBC DB2 ADB {WDB <= 15595} ).\n");
    printf("Copyright(c) 2022 Carlos Ramzuel - Tlaxcala, Mexico.\n");
    getch();
    return 0;
}
