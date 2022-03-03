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
class GenericBinaryFileInfo
{
    public:
        const char *FileName = NULL;
        long FileSize = 0;
        unsigned int HeaderSize = 0;
        unsigned int TotalRecords = 0;
        unsigned int TotalFields = 0;
        unsigned int RecordSize = 0;
        unsigned int StringSize = 0;
        long DataBytes = 0;
        long StringBytes = 0;
        long UnkBytes = 0;
        unsigned char *DataTable = NULL;
        unsigned char *StringTable = NULL;
        unsigned int FormatedTotalFields = 0;
        unsigned int FormatedRecordSize = 0;
};

class GenericBinaryDataAccessor
{
    public:
        GenericBinaryDataAccessor(GenericBinaryFileInfo value) : GenericInfo(value) {}

        vector<enumFieldTypes> FieldTypes;
        bool isFormated() { return !FieldTypes.empty(); }
        void SetFieldTypesToNONE()
        {
            FieldTypes.clear();
            for (unsigned int x = 0; x < GenericInfo.TotalFields; x++)
                FieldTypes.push_back(type_NONE);
        }
        void SetFieldsOffset()
        {
            _fieldsOffset = new unsigned int[GenericInfo.TotalFields];
            _fieldsOffset[0] = 0;
            for (unsigned int i = 1; i < GenericInfo.TotalFields; ++i)
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
                const char *GetString(size_t FieldID) const { return reinterpret_cast<char*>(_info.GenericInfo.StringTable + GetUInt(FieldID)); }
            private:
                RecordAccessor(GenericBinaryDataAccessor &info, unsigned char *data) : _data(data), _info(info) { }
                unsigned char *_data;
                GenericBinaryDataAccessor &_info;
                friend class GenericBinaryDataAccessor;
        };
        unsigned int GetOffset(size_t FieldID) const { return (_fieldsOffset != NULL && FieldID < GenericInfo.TotalFields) ? _fieldsOffset[FieldID] : 0; }
        RecordAccessor GetRecord(size_t  RecordID) { return RecordAccessor(*this, GenericInfo.DataTable + RecordID * GenericInfo.RecordSize); }
    protected:
        unsigned int *_fieldsOffset;
        unsigned int _recordOffset;
        GenericBinaryFileInfo GenericInfo;
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
            // Verificamos que al menos tenga de tama�o 20 bytes
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
