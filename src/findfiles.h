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
        void FileToFind(string directory, string filename, string structure, bool recursive, string fileExt, enumFileType FileType = unkFile);
        unsigned int TotalDBCFiles() { return dbcFileNames.size(); }
        unsigned int TotalDB2Files() { return db2FileNames.size(); }
        unsigned int TotalADBFiles() { return adbFileNames.size(); }
        unsigned int TotalWDBFiles() { return wdbFileNames.size(); }
        unsigned int TotalUNKFiles() { return unkFileNames.size(); }
        void PrintfDBCFiles() {
            for (auto current = dbcFileNames.begin(); current != dbcFileNames.end(); current++)
                sLog->WriteLog("DBC File Added: %s\n", current->first.c_str());
        }
        void PrintfDB2Files() {
            for (auto current = db2FileNames.begin(); current != db2FileNames.end(); current++)
                sLog->WriteLog("DB2 File Added: %s\n", current->first.c_str());
        }
        void PrintfADBFiles() {
            for (auto current = adbFileNames.begin(); current != adbFileNames.end(); current++)
                sLog->WriteLog("ADB File Added: %s\n", current->first.c_str());
        }
        void PrintfWDBFiles() {
            for (auto current = wdbFileNames.begin(); current != wdbFileNames.end(); current++)
                sLog->WriteLog("WDB File Added: %s\n", current->first.c_str());
        }
        void PrintfUNKFiles() {
            for (auto current = unkFileNames.begin(); current != unkFileNames.end(); current++)
                sLog->WriteLog("Unknown File Added: %s\n", current->first.c_str());
        }
        bool ListEmpty() { return dbcFileNames.empty() && db2FileNames.empty() && adbFileNames.empty() && wdbFileNames.empty() && unkFileNames.empty(); }
    private:
        void AddFileToListIfNotExist(string fileName, structFile File)
        {
            auto Found = unkFileNames.find(fileName);
            if (Found != unkFileNames.end())
                return;

            switch (File.Type)
            {
                case dbcFile:
                    dbcFileNames.insert(pair<string, structFile>(fileName, File));
                    break;
                case db2File:
                    db2FileNames.insert(pair<string, structFile>(fileName, File));
                    break;
                case adbFile:
                    adbFileNames.insert(pair<string, structFile>(fileName, File));
                    break;
                case wdbFile:
                    wdbFileNames.insert(pair<string, structFile>(fileName, File));
                    break;
                default:
                    unkFileNames.insert(pair<string, structFile>(fileName, File));
                    break;
            }
        }
    protected:
        map<string, structFile> dbcFileNames;
        map<string, structFile> db2FileNames;
        map<string, structFile> adbFileNames;
        map<string, structFile> wdbFileNames;
        map<string, structFile> unkFileNames;
};

#define sFindFiles CSingleton<FindFiles>::Instance()
#endif
