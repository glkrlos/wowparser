#include "..\revision.h"
#include "pch.h"
#include "shared.h"
#include "dirent.h"
#include "tinyxml2.h"

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

bool LoadConfiguarionFile()
{
    XMLdoc.LoadFile("wowparser3.xml");

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

    for (fileElement; fileElement; fileElement = fileElement->NextSiblingElement("file"))
    {
        const char *_directoryName = fileElement->Attribute("directory");
        string DirectoryName = _directoryName ? _directoryName : "";
        DirectoryName = DirectoryName.empty() ? "." : DirectoryName;

        const char *_fileExtension = fileElement->Attribute("extension");
        string FileExtension = _fileExtension ? _fileExtension : "";
        bool Extension = FileExtension.empty() ? false : true;

        bool isRecursive = false;
        // si el valor de recursive no esta establecido o es un valor incorrecto entonces ponemos que recursive is not set
        bool RecursiveIsSet = fileElement->QueryBoolAttribute("recursive", &isRecursive) ? false : true;

        // Si hay extension no nos intereza el nombre ni el formato
        if (Extension)
        {
            if (!RecursiveIsSet)
                isRecursive = true;
            
            AddFilesToList(DirectoryName, "", "", isRecursive, FileExtension);
            continue;
        }

        const char *_fileName = fileElement->Attribute("name");
        string FileName = _fileName ? _fileName : "";
        bool Name = FileName.empty() ? false : true;

        // Si no hay nombre continuamos
        if (!Name)
            continue;

        const char *_fileFormat = fileElement->Attribute("format");
        string FileFormat = _fileFormat ? _fileFormat : "";
        AddFilesToList(DirectoryName, FileName, FileFormat, isRecursive, "");
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
    }

    printf("\n\nWoWParser Version 3.0 BETA (Revision: %s)\tHash: %s\n", _REVISION, _HASH);
    printf("\t\t\t\t\t\tDate: %s\n", _DATE);
    printf("\nTool to Parse World of Warcraft files (DBC DB2 ADB WDB).\n");
    printf("Copyright(c) 2022 Carlos Ramzuel - Tlaxcala, Mexico.\n");

    _getch();
    return 0;
}