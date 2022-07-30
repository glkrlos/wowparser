#ifndef _FINDFILES_H_
#define _FINDFILES_H_

#include "pch.h"
#include "shared.h"
#include "log.h"

#ifdef _WIN32
    #include "win32/dirent.h"
#else
    #include <dirent.h>
#endif

struct structFile
{
    enumFileType Type;
    string Structure;
    bool isRecursivelySearched;
    bool isSearchedByExtension;
    unsigned int XMLFileID;
};

class FindFiles
{
    public:
        FindFiles()
        {
            countFiles.clear();

            /// Se tiene que crear cada registro para evitar un crash y establecer el contador a 0 que no existan errores de conteo
            for (unsigned int x = 0; x < totalFileTypes; x++)
            {
                countFiles.push_back(x);
                countFiles[x] = 0;
            }
        }
        const char *GetFileExtensionByFileType(enumFileType eFT)
        {
            switch (eFT)
            {
                case dbcFile: return "DBC";
                    break;
                case db2File: return "DB2";
                    break;
                case adbFile: return "ADB";
                    break;
                case wdbFile: return "WDB";
                    break;
                case csvFile: return "CSV";
                    break;
                default: return "Unknown";
                    break;
            }
        }
        /* 
            Obtiene el typo de archivo por extension
            @string = Debe contener una cadena de texto
        */
        enumFileType GetFileTypeByExtension(string FileName)
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
        void FileToFind(string directory, string filename, string structure, bool recursive, string fileExt, unsigned int xmlFileID = 0);
        void PrintFileNamesByFileType()
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
                            sLog->WriteLog("Added to list '%i' %s file%s using recursive mode.\n", countFiles[current->second.Type], GetFileExtensionByFileType(current->second.Type), countFiles[current->second.Type] > 1 ? "s" : "");
                            First = false;
                        }

                        sLog->WriteLog("%s File Added: %s\n", GetFileExtensionByFileType(current->second.Type), current->first.c_str());
                    }
                }
            }
        }
        bool ListEmpty() { return fileNames.empty(); }
    private:
        /*
            Agrega archivos indexados por nombre y solo inserta un nuevo valor si el nombre no existe
        */
        void AddFileToListIfNotExist(string fileName, structFile File)
        {
            auto Found = fileNames.find(fileName);
            if (Found != fileNames.end())
                return;

            fileNames.insert(pair<string, structFile>(fileName, File));

            // Contamos el numero de registros de cada tipo
            countFiles[File.Type]++;

            return;
        }
    protected:
        map<string, structFile> fileNames;
        vector<unsigned int> countFiles;
};

#define sFindFiles CSingleton<FindFiles>::Instance()
#endif
