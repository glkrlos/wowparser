#ifndef WOWPARSER_CSV_SHARED_H
#define WOWPARSER_CSV_SHARED_H

#include "../shared.h"

namespace Csv {
    bool ExtractFields(string originalText, map<unsigned int, string> &mapFields);
    const char* GetFieldTypeName(enumFieldTypes fieldType);
    bool CheckFieldValue(unsigned int fieldID, enumFieldTypes fieldType, string fieldValue, unsigned int recordID);
} // Csv

#endif //WOWPARSER_CSV_SHARED_H
