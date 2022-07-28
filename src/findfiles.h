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
};

class FindFiles
{
    public:
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
        void FileToFind(string directory, string filename, string structure, bool recursive, string fileExt);
        unsigned int TotalDBCFiles() { return dbcFileNames.size(); }
        unsigned int TotalDB2Files() { return db2FileNames.size(); }
        unsigned int TotalADBFiles() { return adbFileNames.size(); }
        unsigned int TotalWDBFiles() { return wdbFileNames.size(); }
        unsigned int TotalCSVFiles() { return csvFileNames.size(); }
        unsigned int TotalUNKFiles() { return unkFileNames.size(); }
        void PrintDBCFiles() {
            for (auto current = dbcFileNames.begin(); current != dbcFileNames.end(); current++)
                sLog->WriteLog("DBC File Added: %s\n", current->first.c_str());
        }
        void PrintDB2Files() {
            for (auto current = db2FileNames.begin(); current != db2FileNames.end(); current++)
                sLog->WriteLog("DB2 File Added: %s\n", current->first.c_str());
        }
        void PrintADBFiles() {
            for (auto current = adbFileNames.begin(); current != adbFileNames.end(); current++)
                sLog->WriteLog("ADB File Added: %s\n", current->first.c_str());
        }
        void PrintWDBFiles() {
            for (auto current = wdbFileNames.begin(); current != wdbFileNames.end(); current++)
                sLog->WriteLog("WDB File Added: %s\n", current->first.c_str());
        }
        void PrintCSVFiles() {
            for (auto current = csvFileNames.begin(); current != csvFileNames.end(); current++)
                sLog->WriteLog("CSV File Added: %s\n", current->first.c_str());
        }
        void PrintUNKFiles() {
            for (auto current = unkFileNames.begin(); current != unkFileNames.end(); current++)
                sLog->WriteLog("Unknown File Added: %s\n", current->first.c_str());
        }
        bool ListEmpty() { return dbcFileNames.empty() && db2FileNames.empty() && adbFileNames.empty() && wdbFileNames.empty() && csvFileNames.empty() && unkFileNames.empty(); }
    private:
        void AddFileToListIfNotExist(string fileName, structFile File)
        {
            switch (File.Type)
            {
                case dbcFile:
                {
                    auto Found = dbcFileNames.find(fileName);
                    if (Found != dbcFileNames.end())
                        break;

                    dbcFileNames.insert(pair<string, structFile>(fileName, File));
                    break;
                }
                case db2File:
                {
                    auto Found = db2FileNames.find(fileName);
                    if (Found != db2FileNames.end())
                        break;

                    db2FileNames.insert(pair<string, structFile>(fileName, File));
                    break;
                }
                case adbFile:
                {
                    auto Found = adbFileNames.find(fileName);
                    if (Found != adbFileNames.end())
                        break;

                    adbFileNames.insert(pair<string, structFile>(fileName, File));
                    break;
                }
                case wdbFile:
                {
                    auto Found = wdbFileNames.find(fileName);
                    if (Found != wdbFileNames.end())
                        break;

                    wdbFileNames.insert(pair<string, structFile>(fileName, File));
                    break;
                }
                case csvFile:
                {
                    auto Found = csvFileNames.find(fileName);
                    if (Found != csvFileNames.end())
                        break;

                    csvFileNames.insert(pair<string, structFile>(fileName, File));
                    break;
                }
                default:
                {
                    auto Found = unkFileNames.find(fileName);
                    if (Found != unkFileNames.end())
                        break;

                    unkFileNames.insert(pair<string, structFile>(fileName, File));
                    break;
                }
            }
        }
    protected:
        map<string, structFile> dbcFileNames;
        map<string, structFile> db2FileNames;
        map<string, structFile> adbFileNames;
        map<string, structFile> wdbFileNames;
        map<string, structFile> csvFileNames;
        map<string, structFile> unkFileNames;
};

#define sFindFiles CSingleton<FindFiles>::Instance()
#endif
