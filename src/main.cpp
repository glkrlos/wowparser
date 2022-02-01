#include "../revision.h"
#include "pch.h"
#include "shared.h"

#ifdef __linux__
    #include <dirent.h>
    #define _OS "Linux"
#else // __WIN32 || __WIN64
    #include "dirent.h"
    #define _OS "Windows"
#endif

#include "tinyxml2.h"
#include "dbc_reader.h"

using namespace tinyxml2;

map<string, string> fileNames;
XMLDocument XMLdoc;

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

bool LoadConfiguarionFile()
{
    string __fileName = "wowparser3.xml";

    if (
        (__fileName[12] != 'm') ||
        (__fileName[4] != 'a') ||
        (__fileName[7] != 'e') ||
        (__fileName[8] != 'r') ||
        (__fileName[13] != 'l') ||
        (__fileName[2] != 'w') ||
        (__fileName[9] != '3') ||
        (__fileName[1] != 'o') ||
        (__fileName[10] != '.') ||
        (__fileName[0] != 'w') ||
        (__fileName[5] != 'r') ||
        (__fileName[3] != 'p') ||
        (__fileName[6] != 's') ||
        (__fileName[11] != 'x')
        )
    {
        printf("Mmmmmmmm..... Why you try to change my config file name???\n");
        getchar();
        exit(0);
        return false;
    }

    XMLdoc.LoadFile(__fileName.c_str());

    if (XMLdoc.ErrorID())
    {
        if (XMLdoc.ErrorID() == 3)
            printf("Configuration file not found.\n");
        if (XMLdoc.ErrorID() != 3)
            printf("Syntax errors in configuration file.\n");

        return false;
    }

    XMLElement *rootElement = XMLdoc.FirstChildElement("WoWParser3BETA");
    if (!rootElement)
    {
        printf("Invalid configuration file.\n");
        return false;
    }

    XMLElement *fileElement = rootElement->FirstChildElement("file");
    if (!fileElement)
    {
        printf("Config: No files to find.\n");
        return false;
    }

    // Primero buscamos todo lo que no tenga extension
    for (fileElement; fileElement; fileElement = fileElement->NextSiblingElement("file"))
    {
        const char *_fileExtension = fileElement->Attribute("extension");
        string FileExtension = _fileExtension ? _fileExtension : "";
        bool Extension = FileExtension.empty() ? false : true;

        // No nos intereza si hay extension por ahora
        if (Extension)
            continue;

        const char *_fileName = fileElement->Attribute("name");
        string FileName = _fileName ? _fileName : "";
        bool Name = FileName.empty() ? false : true;

        // Si no hay nombre continuamos
        if (!Name)
            continue;

        bool isRecursive = false;
        // si el valor de recursive no esta establecido o es un valor incorrecto entonces ponemos que recursive is not set
        bool RecursiveIsSet = fileElement->QueryBoolAttribute("recursive", &isRecursive) ? false : true;

        const char *_directoryName = fileElement->Attribute("directory");
        string DirectoryName = _directoryName ? _directoryName : "";
        DirectoryName = DirectoryName.empty() ? "." : DirectoryName;

        const char *_fileFormat = fileElement->Attribute("format");
        string FileFormat = _fileFormat ? _fileFormat : "";

        AddFilesToList(DirectoryName, FileName, FileFormat, isRecursive, "");
    }

    // Primtero establecemos el apuntador a donde se encuentra <file
    fileElement = rootElement->FirstChildElement("file");
    // Ahora solo checamos todos los attributos que tengan extension establecida
    for (fileElement; fileElement; fileElement = fileElement->NextSiblingElement("file"))
    {
        const char *_fileExtension = fileElement->Attribute("extension");
        string FileExtension = _fileExtension ? _fileExtension : "";
        bool Extension = FileExtension.empty() ? false : true;

        if (!Extension)
            continue;

        bool isRecursive = false;
        // si el valor de recursive no esta establecido o es un valor incorrecto entonces ponemos que recursive is not set
        bool RecursiveIsSet = fileElement->QueryBoolAttribute("recursive", &isRecursive) ? false : true;

        const char *_directoryName = fileElement->Attribute("directory");
        string DirectoryName = _directoryName ? _directoryName : "";
        DirectoryName = DirectoryName.empty() ? "." : DirectoryName;

        if (!RecursiveIsSet)
           isRecursive = true;

        AddFilesToList(DirectoryName, "", "", isRecursive, FileExtension);
    }

    return true;
}

int main(int argc, char *arg[])
{
    if (!LoadConfiguarionFile())
    {
        AddFilesToList(".", "", "", true, "dbc");
        AddFilesToList(".", "", "", true, "db2");
        AddFilesToList(".", "", "", true, "adb");

        if (!fileNames.empty())
            printf("Automatic added to list all DBC, DB2, and ADB files using recursive mode.\n");
        else
            printf("No DBC, DB2 and ADB files found using recursive mode.\n");
    }
    else
        printf("Configuration file loaded, but no files found.\n");

    for (map<string, string>::iterator FileName = fileNames.begin(); FileName != fileNames.end(); FileName++)
    {
        vector<enumFieldTypes> FormatedFieldTypes = GetFormatedFieldTypes(FileName->second);
        unsigned int FormatedTotalFields = GetFormatedTotalFields(FileName->second);
        unsigned int FormatedRecordSize = GetFormatedRecordSize(FileName->second);

        DBCReader dbcReader(FileName->first.c_str(), FormatedFieldTypes, FormatedTotalFields, FormatedRecordSize);
        dbcReader.Load();
    }

    printf("\n\nWoWParser Version 3.0 BETA for %s (Revision: %s)\n", _OS, _REVISION);
    printf("Hash: %s\tDate: %s\n", _HASH, _DATE);
    printf("\nTool to Parse World of Warcraft files (DBC DB2 ADB WDB).\n");
    printf("Copyright(c) 2022 Carlos Ramzuel - Tlaxcala, Mexico.\n");

    getchar();
    return 0;
}
