#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <map>
#include <fstream>
#include <vector>
#include <conio.h>
#include <sstream>

using namespace std;

struct WDBInfo
{
    unsigned int entry;
    unsigned int recordSize;
    unsigned char *recordData;
};

class WDBReader
{
    public:
        WDBReader() :_offset(0) {}
        bool Load();
        bool testpredictwdb();
        unsigned int GetUint() { return *reinterpret_cast<unsigned int*>(GetUnk(4)); }
        const char *GetHeader() { return reinterpret_cast<char*>(GetUnk(4)); }
        unsigned char *GetUnk(unsigned int sizeOfRecord)
        {
            unsigned char *value = new unsigned char[sizeOfRecord];
            value = data + _offset;
            _offset += sizeOfRecord;
            return value;
        }
    private:
        long FileSize;
        unsigned char *data;
        unsigned int _offset;
        vector<WDBInfo> Records;

};

bool WDBReader::Load()
{
    const char *fileName = "questcache.wdb";
    FILE *input = fopen(fileName, "rb");
    if (!input)
    {
        printf("no se pudo abrir\n");
        return false;
    }

    fseek(input, 0, SEEK_END);
    FileSize = ftell(input);
    rewind(input);

    data = new unsigned char[FileSize];
    if (fread(data, FileSize, 1, input) != 1)
        return false;

    fclose(input);

    return true;
}

bool WDBReader::testpredictwdb()
{
    // header, revision, locale, maxrecordsize, unk2, unk3 bytes
    if ((FileSize -= 4 + 4 + 4 + 4 + 4 + 4) < 0)
    {
        printf("File size too small. Are you sure is a WDB file?\n");
        return false;
    }

    const char *header = GetHeader();
    unsigned int revision = GetUint();
    const char *locale = GetHeader();
    unsigned int unk1 = GetUint();
    unsigned int unk2 = GetUint();
    unsigned int unk3 = GetUint();
    printf("header: %c%c%c%c\n", header[3], header[2], header[1], header[0]);
    printf("revision: %u\n", revision);
    printf("locale: %c%c%c%c\n", locale[3], locale[2], locale[1], locale[0]);
    printf("unk1: %u\n", unk1);
    printf("unk2: %u\n", unk2);
    printf("unk3: %u\n", unk3);

    bool isDamaged = false;
    Records.clear();
    while (true)
    {
        // entry, recordSize bytes
        if ((FileSize -= 4 + 4) < 0)
        {
            isDamaged = true;
            break;
        }

        unsigned int entry = GetUint();
        unsigned int recordSize = GetUint();

        if (!entry || !recordSize)
            break;

        // recordData bytes
        if ((FileSize -= recordSize) < 0)
        {
            isDamaged = true;
            break;
        }

        WDBInfo Info;
        Info.entry = entry;
        Info.recordSize = recordSize;
        Info.recordData = GetUnk(recordSize);
        Records.push_back(Info);
    }

    if (isDamaged)
    {
        printf("WDB structure is damaged.\n");
        return false;
    }

    if (Records.empty())
        return false;

    printf("Total Records: '%u'\n", Records.size());

    unsigned int maxStrings = 0;
    for (vector<WDBInfo>::iterator recordID = Records.begin(); recordID != Records.end(); ++recordID)
    {
        unsigned int entry = recordID->entry;
        unsigned int recordSize = recordID->recordSize;
        unsigned char *DATA = recordID->recordData;
        //printf("iterator primero: %u '%u'\n", entry, recordSize);

        // intentanto predecir primero los strings donde estan
        unsigned int countstrings = 0;
        for (unsigned int x = 0; x < recordSize; x++)
        {
            //const char *text = reinterpret_cast<char*>(DATA + x);
            //int value = *reinterpret_cast<int*>(DATA + x);
            //if (value >= 16500000)
            if (DATA[x] >= 0x20 && DATA[x] <= 0x7a) 
            //  !"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz
            // {|}~            <---- no usamos estos que son del 0x7b al 0x7e
            {
                if (DATA[x + 1] >= 0x20 && DATA[x + 1] <= 0x7a)
                {
                    if (DATA[x + 2] >= 0x20 && DATA[x + 2] <= 0x7a)
                    {

                        countstrings++;
                        const char* text = reinterpret_cast<char*>(DATA + x);
                        unsigned int size = strlen(text);
                        //if (countstrings > 9)
                        //{
                            //printf("'%s' pos:%u size:%i\n", text, x, size);
                            //_getch();
                        //}
                        x += size;
                    }
                }
            }
        }

        if (countstrings > maxStrings)
            maxStrings = countstrings;
    }
    printf("%u\n", maxStrings);
    _getch();

    return true;
}


int main(int argc, char *arg[])
{
    WDBReader wdbReader;
    if (wdbReader.Load())
    {
        wdbReader.testpredictwdb();
    }
    _getch();

    return 0;
}