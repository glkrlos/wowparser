#include "config_xml.h"

Config::Config()
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
        getch();
        exit(0);
    }

    XMLdoc.LoadFile(__fileName.c_str());
}

bool Config::LoadConfiguarionFile()
{
    if (XMLdoc.ErrorID())
    {
        if (XMLdoc.ErrorID() == 3)
            sLog->WriteLog("LoadConfiguarionFile(): WARNING: Configuration file not found.\n");
        else
            sLog->WriteLog("LoadConfiguarionFile(): ERROR: Unable to load configuration file. Syntax errors.\n");

        return false;
    }

    XMLElement *rootElement = XMLdoc.FirstChildElement("WoWParser3BETA");
    if (!rootElement)
    {
        sLog->WriteLog("LoadConfiguarionFile(): WARNING: Invalid configuration file.\n");
        return false;
    }

    XMLElement *fileElement = rootElement->FirstChildElement("file");
    if (!fileElement)
    {
        sLog->WriteLog("LoadConfiguarionFile(): WARNING: No files specified.\n");
        return false;
    }

    unsigned int fileID = 1;
    for (fileElement; fileElement; fileElement = fileElement->NextSiblingElement("file"), fileID++)
    {
        const char *_fileExtension = fileElement->Attribute("extension");
        string FileExtension = _fileExtension ? _fileExtension : "";
        bool FileExtensionIsSet = FileExtension.empty() ? false : true;

        const char *_fileName = fileElement->Attribute("name");
        string FileName = _fileName ? _fileName : "";
        bool Name = FileName.empty() ? false : true;

        // Si no hay nombre continuamos
        if (!Name && !FileExtensionIsSet)
        {
            sLog->WriteLog("LoadConfiguarionFile(): WARNING: name attribute can't be empty. Ignoring element '%u'\n", fileID);
            continue;
        }

        bool isRecursive = false;
        // si el valor de recursive no esta establecido o es un valor incorrecto entonces ponemos que recursive is not set
        bool RecursiveAttributeIsSet = fileElement->QueryBoolAttribute("recursive", &isRecursive) ? false : true;

        const char *_directoryName = fileElement->Attribute("directory");
        string DirectoryName = _directoryName ? _directoryName : "";
        DirectoryName = DirectoryName.empty() ? "." : DirectoryName;

        // Si se establecio una extension de archivo y el atributo recursive no esta establecido entonces forzamos dicho modo
        if (!RecursiveAttributeIsSet && FileExtensionIsSet)
            isRecursive = true;

        const char *_fileFormat = fileElement->Attribute("format");
        string FileFormat = _fileFormat ? _fileFormat : "";

        if (!FileExtensionIsSet && !IsValidFormat(FileFormat))
        {
            sLog->WriteLog("LoadConfiguarionFile(): WARNING: For file name '%s' contains an invalid character in format attribute. Ignoring element '%u'\n", FileName.c_str(), fileID);
            continue;
        }

        if (FileExtensionIsSet)
            sLog->WriteLog("LoadConfiguarionFile(): File Element %u:'*.%s'", fileID, FileExtension.c_str());
        else
            sLog->WriteLog("LoadConfiguarionFile(): File Element %u:'%s'", fileID, FileName.c_str());
 
        string tempDirectory = DirectoryName;
        if (!strcmp(tempDirectory.c_str(), "."))
            tempDirectory += "/";

        if (isRecursive)
            sLog->WriteLogNoTime(" Will be able to find it using recursive mode starting on this directory '%s'\n", tempDirectory.c_str());
        else
            sLog->WriteLogNoTime(" Will be able to find it only in this directory '%s'\n", tempDirectory.c_str());

        sFindFiles->FileToFind(DirectoryName, FileName, FileFormat, isRecursive, FileExtensionIsSet ? FileExtension : "");
    }

    sFindFiles->CountTotalFilesByType();

    return true;
}