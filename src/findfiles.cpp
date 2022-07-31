#include "findfiles.h"

FindFiles::FindFiles()
{
    countFiles.clear();

    /// Se tiene que crear cada registro para evitar un crash y establecer el contador a 0 que no existan errores de conteo
    for (unsigned int x = 0; x < totalFileTypes; x++)
    {
        countFiles.push_back(x);
        countFiles[x] = 0;
    }
}

const char *FindFiles::GetFileExtensionByFileType(enumFileType eFT)
{
    switch (eFT)
    {
        case dbcFile: return "dbc";
            break;
        case db2File: return "db2";
            break;
        case adbFile: return "adb";
            break;
        case wdbFile: return "wdb";
            break;
        case csvFile: return "csv";
            break;
        default: return "Unknown";
            break;
    }
}

enumFileType FindFiles::GetFileTypeByExtension(string FileName)
{
    int _tempPosExt = FileName.rfind(".");
    if (_tempPosExt != -1)
    {
        string _tempExt = FileName.substr(_tempPosExt + 1, FileName.size());

        if (!_tempExt.compare("dbc"))
            return dbcFile;
        else if (!_tempExt.compare("db2"))
            return db2File;
        else if (!_tempExt.compare("adb"))
            return adbFile;
        else if (!_tempExt.compare("wdb"))
            return wdbFile;
        else if (!_tempExt.compare("csv"))
            return csvFile;
    }

    return unkFile;
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
            if (!fileExt.empty())
            {
                string _tempFileName = ent->d_name;
                int _tempPosExt = _tempFileName.rfind(".");
                if (_tempPosExt != -1)
                {
                    string _tempExt = _tempFileName.substr(_tempPosExt + 1, _tempFileName.size());
                    if (!strcmp(_tempExt.c_str(), fileExt.c_str()))
                    {
                        structFile File;
                        File.Structure = structure;
                        File.Type = GetFileTypeByExtension(ent->d_name);
                        File.isRecursivelySearched = recursive;
                        File.isSearchedByExtension = true;
                        File.XMLFileID = xmlFileID;
                        AddFileToListIfNotExist(dirName, File);
                    }
                }
            }
            else if (ent->d_name == filename)
            {
                structFile File;
                File.Structure = structure;
                File.Type = GetFileTypeByExtension(ent->d_name);
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
    for (unsigned int x = 0; x < totalFileTypes; x++)
    {
        bool First = true;

        for (auto current = fileNames.begin(); current != fileNames.end(); current++)
        {
            if (x == current->second.Type)
            {
                if (First)
                {
                    sLog->WriteLog("-> '%u' %s file%s added...\n", countFiles[current->second.Type], GetFileExtensionByFileType(current->second.Type), countFiles[current->second.Type] > 1 ? "s" : "");
                    First = false;
                }

                sLog->WriteLog("File: '%s'\n", current->first.c_str());

                if (!current->second.Structure.empty())
                    sLog->WriteLog("---> Structure: %s\n", current->second.Structure.c_str());

                if (current->second.isSearchedByExtension || current->second.XMLFileID)
                {
                    if (current->second.isSearchedByExtension)
                        sLog->WriteLog("---> searched with extension *.%s%s", GetFileExtensionByFileType(current->second.Type), current->second.isRecursivelySearched ? " in recursive mode" : "");

                    if (current->second.XMLFileID)
                    {
                        if (!current->second.isSearchedByExtension)
                            sLog->WriteLog("--->");
 
                        sLog->WriteLogNoTime(" by <file> element %u", current->second.XMLFileID);
                    }

                    sLog->WriteLogNoTime("\n");
                }
            }
        }
    }
}
bool FindFiles::ListEmpty()
{
    return fileNames.empty();
}

void FindFiles::AddFileToListIfNotExist(string fileName, structFile File)
{
    auto Found = fileNames.find(fileName);

    if (Found != fileNames.end() && File.Structure.empty())
        return;

    if (Found != fileNames.end() && !File.Structure.empty())
    {
        countFiles[File.Type]--;
        fileNames.erase(Found);
    }

    fileNames.insert(pair<string, structFile>(fileName, File));

    // Contamos el numero de registros de cada tipo
    countFiles[File.Type]++;

    return;
}