#ifndef _SHARED_H_
#define _SHARED_H_

enum enumFieldTypes
{
    type_NONE   = 0,
    type_STRING = 1,
    type_FLOAT  = 2,
    type_BYTE   = 3,
    type_INT    = 4,
    type_UINT   = 5,
    type_BOOL   = 6,
    type_UBYTE  = 7,
};

enum enumFileFormat
{
    unkFile = 0,
    dbcFile = 1,
    adbFile = 2,
    db2File = 3,
    wdbFile = 4,
    csvFile = 5,
};

template<typename T> string ToStr(T i)
{
    ostringstream buffer;

    buffer << i;

    return buffer.str();
}

// __FUNCSIG__ para imprimir el nombre de la funcion completa y localizar algun fallo si algo no sale bien

// clase generica para leer los archivos binarios como son dbc, adb, y db2
class GenericBinaryReader
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
        long DataBytes = 0;
        long StringBytes = 0;
        long UnkBytes = 0;
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
        class RecordAccessor
        {
            public:
                float GetFloat(size_t FieldID) const { return *reinterpret_cast<float*>(_data + _info.GetOffset(FieldID)); }
                int GetInt(size_t FieldID) const { return *reinterpret_cast<int*>(_data + _info.GetOffset(FieldID)); }
                unsigned int GetUInt(size_t FieldID) const { return *reinterpret_cast<unsigned int*>(_data + _info.GetOffset(FieldID)); }
                unsigned int GetBool(size_t FieldID) const { return GetUInt(FieldID); }
                char GetByte(size_t FieldID) const { return *reinterpret_cast<char *>(_data + _info.GetOffset(FieldID)); }
                const char *GetString(size_t FieldID) const { return reinterpret_cast<char*>(_info.StringTable + GetUInt(FieldID)); }
            private:
                RecordAccessor(GenericBinaryReader &info, unsigned char *data) : _data(data), _info(info) { }
                unsigned char *_data;
                GenericBinaryReader &_info;
                friend class GenericBinaryReader;
        };
        unsigned int GetOffset(size_t FieldID) const { return (_fieldsOffset != NULL && FieldID < TotalFields) ? _fieldsOffset[FieldID] : 0; }
        RecordAccessor GetRecord(size_t  RecordID) { return RecordAccessor(*this, Data + RecordID * RecordSize); }
    protected:
        unsigned int *_fieldsOffset;
        unsigned int _recordOffset;
};

class DBFileReader
{
    public:
        static DBFileReader& Instance()
        {
            static DBFileReader instance;
            return instance;
        }
        enumFileFormat GetFileType(/*mapa con la lista de cada archivo a leer*/)
        {
            // Leemos el archivo
            // Verificamos que al menos tenga de tamaño 20 bytes
            // Comprobamos el header que coincida con WDBC, WADB, WDB2, y los de WDB (TSHW, QTSW, etc...)
            // Regresamos el valor dependiendo del tipo de archivo
        }
        void IncreaseCounter(enumFileFormat FileFormat)
        {
            switch (FileFormat)
            {
                case dbcFile: _dbcFiles++; break;
                case adbFile: _adbFiles++; break;
                case db2File: _db2Files++; break;
                case wdbFile: _wdbFiles++; break;
                case csvFile: _csvFiles++; break;
                case unkFile:
                default:
                    _unkFiles++;
                    break;
            }
        }
    protected:
        unsigned int _dbcFiles = 0;
        unsigned int _adbFiles = 0;
        unsigned int _db2Files = 0;
        unsigned int _wdbFiles = 0;
        unsigned int _csvFiles = 0;
        unsigned int _unkFiles = 0;
};

#define sDBFileReader DBFileReader::Instance()

#endif
