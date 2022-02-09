#include "../revision.h"
#include "pch.h"
#include "shared.h"
#include "config_reader.h"

#include "dbc_reader.h"

map<string, string> fileNames;

void InsertIfFileNotExist(string file, string structure)
{
    map<string, string>::iterator Found = fileNames.find(file);
    if (Found != fileNames.end())
        return;

    fileNames.insert(pair<string, string>(file, structure));
}

void AddFilesToList(string directory, string filename, string structure, bool recursive, string fileExt)
{
    DIR *dir = opendir(directory.c_str());
    struct dirent *ent;

    if (!dir)
        return;

    while ((ent = readdir(dir)) != NULL)
    {
        if (!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, ".."))
            continue;

        string dirName = directory + "/" + ent->d_name;

        if (ent->d_type == DT_REG)
        {
            if (!fileExt.empty())
            {
                string _tempFileName = ent->d_name;
                int _tempPosExt = _tempFileName.rfind(".");
                if (_tempPosExt != -1)
                {
                    string _tempExt = _tempFileName.substr(_tempPosExt + 1, _tempFileName.size());
                    if (!strcmp(_tempExt.c_str(), fileExt.c_str()))
                        InsertIfFileNotExist(dirName, structure);
                }
            }
            else if (ent->d_name == filename)
                InsertIfFileNotExist(dirName, structure);
        }

        if (recursive)
            AddFilesToList(dirName, filename, structure, recursive, fileExt);
    }
    closedir(dir);
}

unsigned int GetFormatedTotalFields(string structure)
{
    return structure.empty() ? 0 : structure.size();
}

unsigned int GetFormatedRecordSize(string structure)
{
    unsigned int RecordSize = 0;

    for (unsigned int x = 0; x < structure.size(); x++)
    {
        switch (structure[x])
        {
            case 'X':   // unk byte
            case 'b':   // byte
                RecordSize += 1;
                break;
            default:
                RecordSize += 4;
                break;
        }
    }

    return RecordSize;
}

vector<enumFieldTypes> GetFormatedFieldTypes(string structure)
{
    vector<enumFieldTypes> fieldTypes;
    for (unsigned int x = 0; x < structure.size(); x++)
    {
        switch (structure[x])
        {
            case 'X':   // unk byte
            case 'b':   // byte
                fieldTypes.push_back(type_BYTE);
                continue;
            case 's':   // string
                fieldTypes.push_back(type_STRING);
                continue;
            case 'f':   // float
                fieldTypes.push_back(type_FLOAT);
                continue;
            case 'd':   // int
            case 'n':   // int
            case 'x':   // unk int
            case 'i':   // int
                fieldTypes.push_back(type_INT);
                continue;
            case 'u':   // unsigned int
                fieldTypes.push_back(type_UINT);
                continue;
            default:
                fieldTypes.push_back(type_NONE);
                continue;
        }
    }

    return fieldTypes;
}

int main(int argc, char *arg[])
{
    FILE *logFile;
    logFile = fopen(WoWParserLogOutPut, "w");
    if (logFile)
        fclose(logFile);

    WriteLog("WoWParser Version 3.0 BETA for %s   (Revision: %s)\n", _OS, _REVISION);
    WriteLog("Hash: %s\tDate: %s\n", _HASH, _DATE);
    WriteLog("Tool to Parse World of Warcraft files (DBC DB2 ADB WDB).\n");
    WriteLog("Copyright(c) 2022 Carlos Ramzuel - Tlaxcala, Mexico.\n");
    WriteLog("\n");
    WriteLog("======================LOG FILE START======================\n");
    
    Config_Reader Cfg;
    if (!Cfg.LoadConfiguarionFile())
    {
        AddFilesToList(".", "", "", true, "dbc");
        unsigned int dbcFilesLoaded = fileNames.size();
        AddFilesToList(".", "", "", true, "db2");
        unsigned int db2FilesLoaded = fileNames.size() > dbcFilesLoaded ? fileNames.size() - dbcFilesLoaded : 0;
        AddFilesToList(".", "", "", true, "adb");
        unsigned int adbFilesLoaded = fileNames.size() > (dbcFilesLoaded + db2FilesLoaded) ? fileNames.size() - dbcFilesLoaded - db2FilesLoaded : 0;

        if (fileNames.empty())
            WriteLogAndPrint("No DBC, DB2 and ADB files found using recursive mode.\n");

        map<string, string>::iterator FileName = fileNames.begin();
        if (dbcFilesLoaded)
        {
            WriteLogAndPrint("Automatic added to list '%i' DBC file%s using recursive mode.\n", dbcFilesLoaded, dbcFilesLoaded > 1 ? "s" : "");
            for (unsigned int x = 0; x < dbcFilesLoaded; x++)
            {
                WriteLog("File: %s\n", FileName->first.c_str());
                FileName++;
            }
        }

        if (db2FilesLoaded)
        {
            WriteLogAndPrint("Automatic added to list '%i' DB2 file%s using recursive mode.\n", db2FilesLoaded, db2FilesLoaded > 1 ? "s" : "");
            for (unsigned int x = 0; x < dbcFilesLoaded; x++)
            {
                WriteLog("File: %s\n", FileName->first.c_str());
                FileName++;
            }
        }

        if (adbFilesLoaded)
        {
            WriteLogAndPrint("Automatic added to list '%i' ADB file%s using recursive mode.\n", adbFilesLoaded, adbFilesLoaded > 1 ? "s" : "");
            for (unsigned int x = 0; x < dbcFilesLoaded; x++)
            {
                WriteLog("File: %s\n", FileName->first.c_str());
                FileName++;
            }
        }
    }
    else
    {
        if (fileNames.empty())
            WriteLogAndPrint("Configuration file loaded, but no files found.\n");
    }

    for (map<string, string>::iterator FileName = fileNames.begin(); FileName != fileNames.end(); FileName++)
    {
        vector<enumFieldTypes> FormatedFieldTypes = GetFormatedFieldTypes(FileName->second);
        unsigned int FormatedTotalFields = GetFormatedTotalFields(FileName->second);
        unsigned int FormatedRecordSize = GetFormatedRecordSize(FileName->second);

        DBCReader dbcReader(FileName->first.c_str(), FormatedFieldTypes, FormatedTotalFields, FormatedRecordSize);
        dbcReader.Load();
    }

    WriteLog("=======================LOG FILE END=======================\n");

    printf("WoWParser Version 3.0 BETA for %s   (Revision: %s)\n", _OS, _REVISION);
    printf("Hash: %s\tDate: %s\n", _HASH, _DATE);
    printf("\nTool to Parse World of Warcraft files (DBC DB2 ADB WDB).\n");
    printf("Copyright(c) 2022 Carlos Ramzuel - Tlaxcala, Mexico.\n");

    getch();
    return 0;
}
