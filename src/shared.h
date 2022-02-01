#ifndef _SHARED_H_
#define _SHARED_H_

#define WoWParserLogOutPut "binaryreader.log"

enum enumFieldTypes
{
    type_NONE   = 0,
    type_STRING = 1,
    type_FLOAT  = 2,
    type_BYTE   = 3,
    type_INT    = 4,
    type_UINT   = 5,
    type_BOOL   = 6,
};

template<typename T> string ToStr(T i)
{
    ostringstream buffer;
    buffer << i;
    return buffer.str();
}

inline void WriteLog(const char* args, ...)
{
    va_list ap;
    va_start(ap, args);
    char outstr[4096];
    vsnprintf(outstr, 4096, args, ap);
    va_end(ap);

    FILE *logFile = fopen(WoWParserLogOutPut, "a");
    if (logFile)
    {
        time_t rawtime;
        struct tm * timeinfo;
        char buffer [80];

        time ( &rawtime );
        timeinfo = localtime ( &rawtime );

        strftime (buffer,80,"%Y-%m-%d %H:%M:%S",timeinfo);

        fprintf(logFile, "%s %s", buffer, outstr);
        fclose(logFile);
    }
}

inline void WriteLogAndPrint(const char* args, ...)
{
    va_list ap;
    va_start(ap, args);
    char outstr[4096];
    vsnprintf(outstr, 4096, args, ap);
    va_end(ap);

    printf("%s", outstr);

    WriteLog(outstr);
}

class BasicFileInfo
{
    public:
        const char *FileName;
        unsigned int HeaderSize;
        unsigned int TotalRecords;
        unsigned int TotalFields;
        unsigned int RecordSize;
        unsigned int StringSize;
        unsigned char *Data;
        unsigned char *StringTable;
        long FileSize;
        unsigned int FormatedTotalFields;
        unsigned int FormatedRecordSize;
        vector<enumFieldTypes> FieldTypes;
        bool isFormated() { return !FieldTypes.empty(); }

        // Record
        void SetFieldTypesToNONE()
        {
            FieldTypes.clear();
            for (unsigned int x = 0; x < TotalFields; x++)
                FieldTypes.push_back(type_NONE);
        }
        void SetFieldsOffset()
        {
            _fieldsOffset = new unsigned int[TotalFields];
            _fieldsOffset[0] = 0;
            for (unsigned int i = 1; i < TotalFields; ++i)
            {
                _fieldsOffset[i] = _fieldsOffset[i - 1];
                if (FieldTypes[i - 1] == type_BYTE)
                    _fieldsOffset[i] += 1;
                else
                    _fieldsOffset[i] += 4;
            }
        }
        float GetFloat(unsigned int FieldID) const { return *reinterpret_cast<float*>(Data + _recordOffset + GetOffset(FieldID)); }
        int GetInt(unsigned int FieldID) const { return *reinterpret_cast<int*>(Data + _recordOffset + GetOffset(FieldID)); }
        unsigned int GetUInt(unsigned int FieldID) const { return *reinterpret_cast<unsigned int*>(Data + _recordOffset + GetOffset(FieldID)); }
        unsigned int GetBool(unsigned int FieldID) const { return GetUInt(FieldID); }
        char GetByte(unsigned int FieldID) const { return *reinterpret_cast<char *>(Data + _recordOffset + GetOffset(FieldID)); }
        const char *GetString(unsigned int FieldID) const { return reinterpret_cast<char*>(StringTable + GetUInt(FieldID)); }
        unsigned int GetOffset(unsigned int FieldID) const { return (_fieldsOffset != NULL && FieldID < TotalFields) ? _fieldsOffset[FieldID] : 0; }
        void GetRecord(unsigned int RecordID) { _recordOffset = RecordID * RecordSize; }
    protected:
        unsigned int *_fieldsOffset;
        unsigned int _recordOffset;
        // Record
};
#endif
