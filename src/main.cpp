#include "../revision.h"
#include "pch.h"
#include "shared.h"
#include "config.h"

int main(int argc, char *arg[])
{
    Config Cfg;
	Cfg.AddFilesToList(".", "", "", true, "dbc");

    printf("WoWParser Version 3.0 for %s (Revision: %s)\n", _OS, _REVISION);
    printf("Hash: %s Date: %s\n", _HASH, _DATE);
    printf("\nTool to Parse World of Warcraft files (DBC DB2 ADB WDB).\n");
    printf("Copyright(c) 2022 Carlos Ramzuel - Tlaxcala, Mexico.\n");

    getch();
    return 0;
}
