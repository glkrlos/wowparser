#include "findfiles.h"

FindFiles::FindFiles()
{
    /*
    countFiles.clear();

    /// Se tiene que crear cada registro para evitar un crash y establecer el contador a 0 que no existan errores de conteo
    for (unsigned int x = 0; x < totalFileTypes; x++)
    {
        countFiles.push_back(x);
        countFiles[x] = 0;
    }
    */
    _fileExtensions["dbc"] = dbcFile;
    _fileExtensions["db2"] = db2File;
    _fileExtensions["adb"] = adbFile;
    _fileExtensions["wdb"] = wdbFile;
    _fileExtensions["csv"] = csvFile;
}

const char *FindFiles::GetFileExtensionByFileType(enumFileType eFT)
{
    for (auto current = _fileExtensions.begin(); current != _fileExtensions.end(); current++)
        if (current->second == eFT)
            return current->first.c_str();

    return "Unknown";
}

enumFileType FindFiles::GetFileTypeByExtension(string FileName)
{
    string _tempExt = FileName.substr(FileName.rfind(".") + 1, FileName.size());

    switch (_fileExtensions[_tempExt])
    {
        case dbcFile: return dbcFile;
        case db2File: return db2File;
        case adbFile: return adbFile;
        case wdbFile: return wdbFile;
        case csvFile: return csvFile;
        default: return unkFile;
    }
}

void FindFiles::FileToFind(string directory, string filename, string structure, bool recursive, string fileExt, unsigned int xmlFileID)
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
            string lowerCaseOriginalFileName = ent->d_name;
            transform(lowerCaseOriginalFileName.begin(), lowerCaseOriginalFileName.end(), lowerCaseOriginalFileName.begin(), ::tolower);
            transform(filename.begin(), filename.end(), filename.begin(), ::tolower);
            transform(structure.begin(), structure.end(), structure.begin(), ::tolower);
            transform(fileExt.begin(), fileExt.end(), fileExt.begin(), ::tolower);

            if (!fileExt.empty())
            {
                if (HaveExtension(lowerCaseOriginalFileName))
                {
                    if (CompareTexts(GetFileExtension(lowerCaseOriginalFileName), fileExt))
                    {
                        structFile File;
                        File.Structure = structure;
                        File.Type = GetFileTypeByExtension(lowerCaseOriginalFileName);
                        File.isRecursivelySearched = recursive;
                        File.isSearchedByExtension = true;
                        File.XMLFileID = xmlFileID;
                        AddFileToListIfNotExist(dirName, File);
                    }
                }
            }
            else if (CompareTexts(lowerCaseOriginalFileName, filename))
            {
                structFile File;
                File.Structure = structure;
                File.Type = GetFileTypeByExtension(lowerCaseOriginalFileName);
                File.isRecursivelySearched = recursive;
                File.isSearchedByExtension = false;
                File.XMLFileID = xmlFileID;
                AddFileToListIfNotExist(dirName, File);
            }
        }

        if (recursive)
            FileToFind(dirName, filename, structure, recursive, fileExt, xmlFileID);
    }
    closedir(dir);
}

void FindFiles::PrintAllFileNamesByFileType()
{
    unsigned int maxFileIDInXML = 0;

    for (auto current = fileNames.begin(); current != fileNames.end(); current++)
        if (current->second.XMLFileID > maxFileIDInXML)
            maxFileIDInXML = current->second.XMLFileID;

    for (unsigned int currentFileID = 0; currentFileID <= maxFileIDInXML; currentFileID++)
    {
        for (unsigned int x = 0; x < totalFileTypes; x++)
        {
            unsigned int countCurrentFiles = 0;

            for (auto current = fileNames.begin(); current != fileNames.end(); current++)
            {
                if (current->second.Type != x || current->second.XMLFileID != currentFileID)
                    continue;
                
                countCurrentFiles++;
            }

            bool First = true;

            for (auto current = fileNames.begin(); current != fileNames.end(); current++)
            {
                if (current->second.Type != x || current->second.XMLFileID != currentFileID)
                    continue;

                if (First)
                {
                    sLog->WriteLog("-> '%u' %s file%s added", countCurrentFiles, GetFileExtensionByFileType(current->second.Type), countCurrentFiles > 1 ? "s" : "");
                    if (current->second.isSearchedByExtension)
                        sLog->WriteLogNoTime(" with extension *.%s%s", GetFileExtensionByFileType(current->second.Type), current->second.isRecursivelySearched ? " in recursive mode" : "");

                    if (current->second.XMLFileID)
                        sLog->WriteLogNoTime(" by <file> element %u", current->second.XMLFileID);

                    sLog->WriteLogNoTime("\n");

                    First = false;
                }

                sLog->WriteLog("File: '%s'", current->first.c_str());

                if (!current->second.Structure.empty())
                    sLog->WriteLogNoTime(", Structure: '%s'", current->second.Structure.c_str());

                sLog->WriteLogNoTime("\n");
            }

        }

    }
}

bool FindFiles::ListEmpty()
{
    return fileNames.empty();
}

bool FindFiles::HaveExtension(string fileName)
{
    return fileName.rfind(".") != -1;
}

bool FindFiles::CompareTexts(string txt1, string txt2)
{
    return !txt1.compare(txt2);
}

string FindFiles::GetFileExtension(string fileName)
{
    return fileName.substr(fileName.rfind(".") + 1, fileName.size());
}

void FindFiles::AddFileToListIfNotExist(string fileName, structFile File)
{
    auto Found = fileNames.find(fileName);

    if (Found != fileNames.end())
    {
        if (!File.Structure.empty())
            Found->second = File;

        return;
    }

    fileNames.insert(pair<string, structFile>(fileName, File));

    // Contamos el numero de registros de cada tipo
    // countFiles[File.Type]++;

    return;
}