#include "findfiles.h"

cFindFiles::cFindFiles()
{
}

enumFileType cFindFiles::GetFileTypeByExtension(string FileName)
{
    string _tempExt = FileName.substr(FileName.rfind(".") + 1, FileName.size());

    if (Shared->CompareTexts(_tempExt, "dbc"))
        return dbcFile;
    else if (Shared->CompareTexts(_tempExt, "db2"))
        return db2File;
    else if (Shared->CompareTexts(_tempExt, "adb"))
        return adbFile;
    else if (Shared->CompareTexts(_tempExt, "wdb"))
        return wdbFile;
    else if (Shared->CompareTexts(_tempExt, "csv"))
        return csvFile;
    else
        return unkFile;
}

void cFindFiles::FileToFind(string directory, string filename, string structure, bool recursive, string fileExt, outputFormat outFormats, unsigned int xmlFileID)
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
            Shared->ToLowerCase(lowerCaseOriginalFileName);
            Shared->ToLowerCase(filename);
            Shared->ToLowerCase(structure);
            Shared->ToLowerCase(fileExt);

            if (!fileExt.empty())
            {
                if (HaveExtension(lowerCaseOriginalFileName))
                {
                    if (Shared->CompareTexts(GetFileExtension(lowerCaseOriginalFileName), fileExt))
                    {
                        structXMLFileInfo XMLFileInfo;
                        XMLFileInfo.FileName = dirName;
                        XMLFileInfo.Structure = structure;
                        XMLFileInfo.Type = GetFileTypeByExtension(lowerCaseOriginalFileName);
                        XMLFileInfo.isRecursivelySearched = recursive;
                        XMLFileInfo.isSearchedByExtension = true;
                        XMLFileInfo.XMLFileID = xmlFileID;
                        XMLFileInfo.FormatedFieldTypes = GetFormatedFieldTypes(structure);
                        XMLFileInfo.FormatedRecordSize = GetFormatedRecordSize(structure);
                        XMLFileInfo.FormatedTotalFields = GetFormatedTotalFields(structure);
                        XMLFileInfo.outputFormats = outFormats;

                        if (!XMLFileInfo.outputFormats.isSetToCSV)
                        {
                            if (GetFileTypeByExtension(lowerCaseOriginalFileName) != csvFile)
                                XMLFileInfo.outputFormats.ToCSV = false;
                            else
                                XMLFileInfo.outputFormats.ToCSV = false;
                        }

                        XMLFileInfo.outputFormats.ToDBC = !XMLFileInfo.outputFormats.isSetToDBC ? false : XMLFileInfo.outputFormats.ToDBC;
                        XMLFileInfo.outputFormats.ToSQL = !XMLFileInfo.outputFormats.isSetToSQL ? true : XMLFileInfo.outputFormats.ToSQL;
                        AddFileToListIfNotExist(dirName, XMLFileInfo);
                    }
                }
            }
            else if (Shared->CompareTexts(lowerCaseOriginalFileName, filename))
            {
                structXMLFileInfo XMLFileInfo;
                XMLFileInfo.FileName = dirName;
                XMLFileInfo.Structure = structure;
                XMLFileInfo.Type = GetFileTypeByExtension(lowerCaseOriginalFileName);
                XMLFileInfo.isRecursivelySearched = recursive;
                XMLFileInfo.isSearchedByExtension = false;
                XMLFileInfo.XMLFileID = xmlFileID;
                XMLFileInfo.FormatedFieldTypes = GetFormatedFieldTypes(structure);
                XMLFileInfo.FormatedRecordSize = GetFormatedRecordSize(structure);
                XMLFileInfo.FormatedTotalFields = GetFormatedTotalFields(structure);
                XMLFileInfo.outputFormats = outFormats;

                if (!XMLFileInfo.outputFormats.isSetToCSV)
                {
                    if (GetFileTypeByExtension(lowerCaseOriginalFileName) != csvFile)
                        XMLFileInfo.outputFormats.ToCSV = false;
                    else
                        XMLFileInfo.outputFormats.ToCSV = false;
                }

                XMLFileInfo.outputFormats.ToDBC = !XMLFileInfo.outputFormats.isSetToDBC ? false : XMLFileInfo.outputFormats.ToDBC;
                XMLFileInfo.outputFormats.ToSQL = !XMLFileInfo.outputFormats.isSetToSQL ? true : XMLFileInfo.outputFormats.ToSQL;
                AddFileToListIfNotExist(dirName, XMLFileInfo);
            }
        }

        if (recursive)
            FileToFind(dirName, filename, structure, recursive, fileExt, outFormats, xmlFileID);
    }
    closedir(dir);
}

void cFindFiles::PrintAllFileNamesByFileType()
{
    if (fileNames.empty())
        return;

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
                    Log->WriteLog("->%s '%u' %s file%s added", current->second.Type == unkFile ? "(WARNING)" : "", countCurrentFiles, Shared->GetFileExtensionByFileType(current->second.Type), countCurrentFiles > 1 ? "s" : "");

                    if (current->second.isSearchedByExtension)
                        Log->WriteLogNoTime(" with extension *.%s%s", current->second.Type == unkFile ? GetFileExtension(current->first).c_str() : Shared->GetFileExtensionByFileType(current->second.Type), current->second.isRecursivelySearched ? " in recursive mode" : "");

                    if (current->second.XMLFileID)
                        Log->WriteLogNoTime(" by <file> element '%u'", current->second.XMLFileID);

                    if (current->second.isSearchedByExtension)
                        Log->WriteLogNoTime(", and they will pass to predicted mode");

                    if (current->second.outputFormats.ToCSV || current->second.outputFormats.ToDBC || current->second.outputFormats.ToSQL)
                    {
                        Log->WriteLogNoTime(" with output to");

                        unsigned int contamostotalsalidas = 0;

                        if (current->second.outputFormats.ToCSV)
                        {
                            Log->WriteLogNoTime(" CSV");
                            contamostotalsalidas++;
                        }

                        if (current->second.outputFormats.ToDBC)
                        {
                            if (current->second.outputFormats.ToCSV && current->second.outputFormats.ToSQL)
                                Log->WriteLogNoTime(",");
                            else if (current->second.outputFormats.ToCSV && !current->second.outputFormats.ToSQL)
                                Log->WriteLogNoTime(" and");

                            Log->WriteLogNoTime(" DBC");

                            contamostotalsalidas++;
                        }

                        if (current->second.outputFormats.ToSQL)
                        {
                            if (current->second.outputFormats.ToCSV || current->second.outputFormats.ToDBC)
                                Log->WriteLogNoTime(" and");

                            Log->WriteLogNoTime(" SQL");
                            contamostotalsalidas++;
                        }

                        Log->WriteLogNoTime(" file format%s", contamostotalsalidas > 1 ? "s" : "");
                    }
                    else
                        Log->WriteLogNoTime(" with file information only");

                    Log->WriteLogNoTime(".\n");

                    First = false;
                }

                Log->WriteLog("File: '%s'", current->second.FileName.c_str());

                if (!current->second.Structure.empty())
                    Log->WriteLogNoTime(", Structure: '%s'", current->second.Structure.c_str());

                Log->WriteLogNoTime("\n");
            }

        }

    }
}

bool cFindFiles::ListEmpty()
{
    return fileNames.empty();
}

bool cFindFiles::HaveExtension(string fileName)
{
    return fileName.rfind(".") != -1;
}

string cFindFiles::GetFileExtension(string fileName)
{
    return fileName.substr(fileName.rfind(".") + 1, fileName.size());
}

void cFindFiles::AddFileToListIfNotExist(string fileName, structXMLFileInfo File)
{
    auto Found = fileNames.find(fileName);

    if (Found != fileNames.end())
    {
        if (!File.Structure.empty())
            Found->second = File;

        return;
    }

    fileNames.insert(pair<string, structXMLFileInfo>(fileName, File));

    return;
}