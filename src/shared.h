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
