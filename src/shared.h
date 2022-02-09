#ifndef _SHARED_H_
#define _SHARED_H_

#ifdef __WIN32
    #include <time.h>
#endif

#define WoWParserLogOutPut "wowparser3.log"

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
        class Record
        {
            public:
                float GetFloat(size_t FieldID) const { return *reinterpret_cast<float*>(_data + _info.GetOffset(FieldID)); }
                int GetInt(size_t FieldID) const { return *reinterpret_cast<int*>(_data + _info.GetOffset(FieldID)); }
                unsigned int GetUInt(size_t FieldID) const { return *reinterpret_cast<unsigned int*>(_data + _info.GetOffset(FieldID)); }
                unsigned int GetBool(size_t FieldID) const { return GetUInt(FieldID); }
                char GetByte(size_t FieldID) const { return *reinterpret_cast<char *>(_data + _info.GetOffset(FieldID)); }
                const char *GetString(size_t FieldID) const { return reinterpret_cast<char*>(_info.StringTable + GetUInt(FieldID)); }
            private:
                Record(BasicFileInfo &info, unsigned char *data) : _data(data), _info(info) { }
                unsigned char *_data;
                BasicFileInfo &_info;
                friend class BasicFileInfo;
        };
        unsigned int GetOffset(size_t FieldID) const { return (_fieldsOffset != NULL && FieldID < TotalFields) ? _fieldsOffset[FieldID] : 0; }
        Record GetRecord(size_t  RecordID) { return Record(*this, Data + RecordID * RecordSize); }
    protected:
        unsigned int *_fieldsOffset;
        unsigned int _recordOffset;
};
#endif
