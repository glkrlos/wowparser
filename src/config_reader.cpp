#include "config_reader.h"

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

bool Config_Reader::IsValidFormat(string structure)
{
    for (unsigned int x = 0; x < structure.size(); x++)
    {
        switch (structure[x])
        {
            case 'X':   // unk byte
            case 'b':   // byte
            case 's':   // string
            case 'f':   // float
            case 'd':   // int
            case 'n':   // int
            case 'x':   // unk int
            case 'i':   // int
            case 'u':   // unsigned int
                break;
            default:
                return false;
        }
    }
    
    return true;
}

bool Config_Reader::LoadConfiguarionFile()
{
    if (XMLdoc.ErrorID())
    {
        if (XMLdoc.ErrorID() == 3)
            WriteLog("LoadConfiguarionFile(): WARNING: Configuration file not found.\n");
        if (XMLdoc.ErrorID() != 3)
            WriteLog("LoadConfiguarionFile(): WARNING: Unable to load configuration file. Syntax errors.\n");

        return false;
    }

    XMLElement *rootElement = XMLdoc.FirstChildElement("WoWParser3BETA");
    if (!rootElement)
    {
        WriteLog("LoadConfiguarionFile(): WARNING: Invalid configuration file.\n");
        return false;
    }

    XMLElement *fileElement = rootElement->FirstChildElement("file");
    if (!fileElement)
    {
        WriteLog("LoadConfiguarionFile(): WARNING: No files specified.\n");
        return false;
    }

    // Primero buscamos todo lo que no tenga extension
    unsigned int fileID = 1;
    for (fileElement; fileElement; fileElement = fileElement->NextSiblingElement("file"), fileID++)
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
        {
            WriteLog("LoadConfiguarionFile(): WARNING: name attribute can't be empty. Ignoring element '%u'\n", fileID);
            continue;
        }

        bool isRecursive = false;
        // si el valor de recursive no esta establecido o es un valor incorrecto entonces ponemos que recursive is not set
        bool RecursiveIsSet = fileElement->QueryBoolAttribute("recursive", &isRecursive) ? false : true;

        const char *_directoryName = fileElement->Attribute("directory");
        string DirectoryName = _directoryName ? _directoryName : "";
        DirectoryName = DirectoryName.empty() ? "." : DirectoryName;

        const char *_fileFormat = fileElement->Attribute("format");
        string FileFormat = _fileFormat ? _fileFormat : "";

        if (!IsValidFormat(FileFormat))
        {
            WriteLog("LoadConfiguarionFile(): WARNING: For file name '%s' contains an invalid character in format attribute. Ignoring element '%u'\n", FileName.c_str(), fileID);
            continue;
        }

        WriteLog("LoadConfiguarionFile(): File %u:'%s'\n", fileID, FileName.c_str());
 
        string tempDirectory = DirectoryName;
        if (!strcmp(tempDirectory.c_str(), "."))
            tempDirectory += "/";

        if (isRecursive)
            WriteLog("LoadConfiguarionFile(): Will be able to find it using recursive mode starting on directory '%s'\n", tempDirectory.c_str());
        else
            WriteLog("LoadConfiguarionFile(): Will be able to find it only in this directory '%s'\n", tempDirectory.c_str());

        //AddFilesToList(DirectoryName, FileName, FileFormat, isRecursive, "");
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

        //AddFilesToList(DirectoryName, "", "", isRecursive, FileExtension);
    }

    return true;
}
