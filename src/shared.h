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

enum enumFileType
{
    unkFile = 0,
    dbcFile = 1,
    db2File = 2,
    adbFile = 3,
    wdbFile = 4,
    csvFile = 5,
    totalFileTypes = 6,
};

struct structFile
{
    public:
        structFile()
        {
            FileName.clear();
            Type = unkFile;
            Structure.clear();
            FormatedFieldTypes.clear();
        }

        string FileName;
        enumFileType Type;
        string Structure;
        bool isRecursivelySearched = false;
        bool isSearchedByExtension = false;
        unsigned int XMLFileID = 0;
        vector<enumFieldTypes> FormatedFieldTypes;
        unsigned int FormatedTotalFields = 0;
        unsigned int FormatedRecordSize = 0;
};

struct structHeader
{
    char value01[4];
    unsigned char *value02;
    unsigned char *value03;
    unsigned char *value04;
    unsigned char *value05;
    unsigned char *value06;
    unsigned char *value07;
    unsigned char *value08;
    unsigned char *value09;
    unsigned char *value10;
    unsigned char *value11;
    unsigned char *value12;
};

struct structWDBHeader
{
    /// 24 bytes del header + 8 bytes del primer record y su el tamaño del record
    char header[4];
    unsigned int revision;
    char locale[4];
    unsigned int maxRecordSize;
    unsigned int unk1;
    unsigned int unk2;

    /*
        Para cada registro:

        unsigned int entry;
        unsigned int recordSize;
        unsigned char *restOfrecord; <- aqui hay que saber el formato para leerlo
    */

    /*
        BDIW itemcache.wdb -> se abre de forma especial por que dependen de unos bytes las veces que lee otros bytes
        BOMW creaturecache.wdb
        BOGW gameobjectcache.wdb
        BDNW itemnamecache.wdb
        XTIW itemtextcache.wdb
        CPNW npccache.wdb
        XTPW pagetextcache.wdb
        TSQW questcache.wdb
    */
};

struct structDBCADBHeader
{
    /// 20 bytes del header
    char header[4];             // WDBC dbc, WCH2 adb
    unsigned int totalRecords;
    unsigned int totalFields;
    unsigned int recordSize;
    unsigned int stringSize;
};

struct structDB2Header
{
    /// 32 bytes del header o 48 bytes si el build > 12880
    char header[4];             // WDB2 db2
    unsigned int totalRecords;
    unsigned int totalFields;
    unsigned int recordSize;
    unsigned int stringSize;
    unsigned int tableHash;
    unsigned int build;
    unsigned int unk1;

    /// > 12880
    /// int diff = maxIndexDB2 - unk2DB2 + 1;
    /// fseek(input, diff * 4 + diff * 2, SEEK_CUR); // diff * 4: an index for rows, diff * 2: a memory allocation bank
    unsigned int unk2;
    unsigned int maxIndex;
    unsigned int locales;
    unsigned int unk3;
};

template <typename T> class CSingleton
{
    public:
        static T* Instance()
        {
            if (!m_instance.get())
                m_instance = auto_ptr<T>(new T);

            return m_instance.get();
        };
    protected:
        //CSingleton();
        //~CSingleton();
    private:
        //CSingleton(CSingleton const&);
        //CSingleton& operator = (CSingleton const*);
        static auto_ptr<T> m_instance;
};

template <typename T> auto_ptr<T> CSingleton<T>::m_instance;

class cShared
{
    public:
        template <typename T> string ToStr(T i)
        {
            ostringstream buffer;

            buffer << i;

            return buffer.str();
        }
        unsigned int GetFormatedTotalFields(string structure)
        {
            return structure.empty() ? 0 : structure.size();
        }
        inline bool IsValidFormat(string structure)
        {
            for (unsigned int x = 0; x < structure.size(); x++)
            {
                switch (structure[x])
                {
                case 'X':   // unk byte
                case 'b':   // byte
                case 's':   // string
                case 'f':   // float
                case 'd':   // int
                case 'n':   // int
                case 'x':   // unk int
                case 'i':   // int
                case 'u':   // unsigned int
                    break;
                default:
                    return false;
                }
            }

            return true;
        }
        unsigned int GetFormatedRecordSize(string structure)
        {
            unsigned int RecordSize = 0;

            for (unsigned int x = 0; x < structure.size(); x++)
            {
                switch (structure[x])
                {
                    case 'X':   // unk byte
                    case 'b':   // byte
                        RecordSize += 1;
                        break;
                    default:
                        RecordSize += 4;
                        break;
                }
            }

            return RecordSize;
        }
        vector<enumFieldTypes> GetFormatedFieldTypes(string structure)
        {
            vector<enumFieldTypes> fieldTypes;
            for (unsigned int x = 0; x < structure.size(); x++)
            {
                switch (structure[x])
                {
                    case 'X':   // unk byte
                    case 'b':   // byte
                        fieldTypes.push_back(type_BYTE);
                        continue;
                    case 's':   // string
                        fieldTypes.push_back(type_STRING);
                        continue;
                    case 'f':   // float
                        fieldTypes.push_back(type_FLOAT);
                        continue;
                    case 'd':   // int
                    case 'n':   // int
                    case 'x':   // unk int
                    case 'i':   // int
                        fieldTypes.push_back(type_INT);
                        continue;
                    case 'u':   // unsigned int
                        fieldTypes.push_back(type_UINT);
                        continue;
                    default:
                        fieldTypes.push_back(type_NONE);
                        continue;
                }
            }

            return fieldTypes;
        }
        bool CompareTexts(string txt1, string txt2)
        {
            return !txt1.compare(txt2);
        }
};

#define Shared CSingleton<cShared>::Instance()

// __FUNCSIG__ para imprimir el nombre de la funcion completa y localizar algun fallo si algo no sale bien

/*
class classSingleton
{
    public:
        static classSingleton& Instance()
        {
            static classSingleton instance;
            return instance;
        }
    private:
    protected:
};

#define TestClassOfSingleton classSingleton::Instance()
*/
#endif
