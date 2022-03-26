#include "pch.h"
#include "dirent.h"
#include "csv_reader.h"
#include "dbc_writer.h"

vector<string> FileNames;
void AddFilesToList(string directory = ".")
{
    DIR *dir;
    struct dirent *ent;

    if ((dir = opendir(directory.c_str())) != NULL)
    {
        while ((ent = readdir(dir)) != NULL)
        {
            if (!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, ".."))
                continue;

            string dirName = directory + "/" + ent->d_name;
            if (ent->d_type == DT_REG)
            {
                int posExt = dirName.rfind(".");
                string fileExt = dirName.substr(posExt, dirName.size());
                if (!strcmp(fileExt.c_str(), ".csv"))
                    FileNames.push_back(dirName);
            }
            else
                AddFilesToList(dirName);
        }
        closedir(dir);
    }
}

int main(int argc, char *arg[])
{
    AddFilesToList();
    if (FileNames.empty())
        printf("No CSV files found in current directory.\n");
    else
    {
        for (vector<string>::iterator it = FileNames.begin(); it != FileNames.end(); ++it)
        {
            CSV_Reader *Reader = new CSV_Reader(it->c_str());
            if (Reader->LoadCSVFile())
            {
                DBC_Writer *Writer = new DBC_Writer(Reader->ExportFileData());
                Writer->CreateDBC();
                delete Writer;
            }

            delete Reader;
        }
    }

    _getch();
    return 0;
}