#include "module_config_reader.h"

Config_Reader::Config_Reader()
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

bool Config_Reader::LoadConfiguarionFile()
{
    Log->WriteLogAndPrint("-----> Loading Configuration file... ");

    if (XMLdoc.ErrorID())
    {
        if (XMLdoc.ErrorID() == 3)
            Log->WriteLogNoTimeAndPrint("Failed: File not found.\n");
        else
            Log->WriteLogNoTimeAndPrint("Failed: Syntax errors.\n");

        return false;
    }

    XMLElement *rootElement = XMLdoc.FirstChildElement("WoWParser3");
    if (!rootElement)
    {
        Log->WriteLogNoTimeAndPrint("Failed: Invalid XML file.\n");
        return false;
    }

    XMLElement *fileElement = rootElement->FirstChildElement("file");
    if (!fileElement)
    {
        Log->WriteLogNoTimeAndPrint("Failed: No files to parse.\n");
        return false;
    }

    Log->WriteLogNoTimeAndPrint("OK\n");

    Log->WriteLog("\n");
    Log->WriteLog("-----> Checking XML attributes of files to parse...\n");

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
            Log->WriteLog("\t WARNING: name attribute can't be empty in configuration file. Ignoring element number '%u'\n", fileID);
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

        if (!FileExtensionIsSet && !Shared->IsValidFormat(FileFormat))
        {
            Log->WriteLog("\t WARNING: For file name '%s' contains an invalid character in format attribute. Ignoring element '%u'\n", FileName.c_str(), fileID);
            continue;
        }

        string tempDirectory = DirectoryName;
        if (!strcmp(tempDirectory.c_str(), "."))
            tempDirectory += "/";

        FindFiles->FileToFind(DirectoryName, FileName, FileFormat, isRecursive, FileExtensionIsSet ? FileExtension : "", fileID);
    }

    Log->WriteLog("-----> All OK after checking XML attributes of files to parse.\n");

    return true;
}