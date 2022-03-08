#include "config.h"

void Config::AddFilesToList(string directory, string filename, string structure, bool recursive, string fileExt)
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

unsigned int Config::GetFormatedRecordSize(string structure)
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

vector<enumFieldTypes> Config::GetFormatedFieldTypes(string structure)
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
