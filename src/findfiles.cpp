#include "findfiles.h"

void FindFiles::FileToFind(string directory, string filename, string structure, bool recursive, string fileExt, enumFileType FileType)
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
                        File.Type = FileType;
                        AddFileToListIfNotExist(dirName, File);
                    }
                }
            }
            else if (ent->d_name == filename)
            {
                structFile File;
                File.Structure = structure;
                File.Type = FileType;
                AddFileToListIfNotExist(dirName, File);
            }
        }

        if (recursive)
            FileToFind(dirName, filename, structure, recursive, fileExt, FileType);
    }
    closedir(dir);
}