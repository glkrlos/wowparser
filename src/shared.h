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
};

// __FUNCSIG__ para imprimir el nombre de la funcion completa y localizar algun fallo si algo no sale bien

class DBFileReader
{
    public:
        static DBFileReader& Instance()
        {
            static DBFileReader instance;
            return instance;
        }
};

#define sDBFileReader DBFileReader::Instance()

#endif
