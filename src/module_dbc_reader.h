#ifndef _MODULE_DBC_READER_H_
#define _MODULE_DBC_READER_H_

#include "pch.h"

struct structDBCHeader
{
    char header[4];
    unsigned int totalRecords;
    unsigned int totalFields;
    unsigned int recordSize;
    unsigned int stringSize;
};

#endif