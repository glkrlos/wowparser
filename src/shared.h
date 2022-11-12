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

    wdbitemcacheFile = 7,
    wdbcreaturecacheFile = 8,
    wdbgameobjectcacheFile = 9,
    wdbitemnamecacheFile = 10,
    wdbitemtextcacheFile = 11,
    wdbnpccacheFile = 12,
    wdbpagetextcacheFile = 13,
    wdbquestcacheFile = 14,
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
        const char *GetFileExtensionByFileType(enumFileType eFT)
        {
            switch (eFT)
            {
                case dbcFile: return "dbc";
                case db2File: return "db2";
                case adbFile: return "adb";
                case wdbFile: return "wdb";
                case csvFile: return "csv";
                default: return "Unknown";
            }
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
