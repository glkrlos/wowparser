#ifndef _CSV_WRITER_H_
#define _CSV_WRITER_H_

#include "pch.h"
#include "shared.h"
#include "csv_reader.h"

struct structDBCHeader
{
    char headerName[4];
    int totalRecords;
    int totalFields;
    int recordSize;
    int stringSize;
};

class DBC_Writer
{
    public:
        DBC_Writer(FileData *);
        ~DBC_Writer();
        void CreateDBC();
    private:
        FileData *_file;
};

#endif