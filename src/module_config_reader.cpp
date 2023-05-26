#include "module_config_reader.h"

Config_Reader::Config_Reader()
{
    string _fileName = "wowparser3.xml";

    if (
        (_fileName[12] != 'm') ||
        (_fileName[4] != 'a') ||
        (_fileName[7] != 'e') ||
        (_fileName[8] != 'r') ||
        (_fileName[13] != 'l') ||
        (_fileName[2] != 'w') ||
        (_fileName[9] != '3') ||
        (_fileName[1] != 'o') ||
        (_fileName[10] != '.') ||
        (_fileName[0] != 'w') ||
        (_fileName[5] != 'r') ||
        (_fileName[3] != 'p') ||
        (_fileName[6] != 's') ||
        (_fileName[11] != 'x')
        )
    {
        printf("Mmmmmmmm..... Why you try to change my config file name???\n");
        getch();
        exit(0);
    }

    XMLdoc.LoadFile(_fileName.c_str());
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
    for (; fileElement; fileElement = fileElement->NextSiblingElement("file"), fileID++)
    {
        const char *_fileExtension = fileElement->Attribute("extension");
        string FileExtension = _fileExtension ? _fileExtension : "";
        bool FileExtensionIsSet = !FileExtension.empty();

        const char *_fileName = fileElement->Attribute("name");
        string FileName = _fileName ? _fileName : "";
        bool Name = !FileName.empty();

        // Si no hay nombre continuamos
        if (!Name && !FileExtensionIsSet)
        {
            Log->WriteLog("\t WARNING: name attribute can't be empty in configuration file. Ignoring element number '%u'\n", fileID);
            continue;
        }

        bool isRecursive = false;
        // si el valor de recursive no esta establecido o es un valor incorrecto entonces ponemos que recursive is not set
        bool RecursiveAttributeIsSet = fileElement->QueryBoolAttribute("recursive", &isRecursive) == 0;

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
            Log->WriteLog("\t WARNING: For file name '%s' contains an invalid character in format attribute. Ignoring element '%u'\n", FileName.c_str(), fileID);
            continue;
        }

        string tempDirectory = DirectoryName;
        if (!strcmp(tempDirectory.c_str(), "."))
            tempDirectory += "/";

        outputFormat outFormats;
        outFormats.isSetToCSV = fileElement->QueryBoolAttribute("ToCSV", &outFormats.ToCSV) == 0;
        outFormats.isSetToDBC = fileElement->QueryBoolAttribute("ToDBC", &outFormats.ToDBC) == 0;
        outFormats.isSetToSQL = fileElement->QueryBoolAttribute("ToSQL", &outFormats.ToSQL) == 0;

        FindFiles->FileToFind(DirectoryName, FileName, FileFormat, isRecursive, FileExtensionIsSet ? FileExtension : "", outFormats, fileID);
    }

    Log->WriteLog("-----> All OK after checking XML attributes of files to parse.\n");

    return true;
}