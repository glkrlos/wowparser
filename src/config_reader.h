#ifndef _CONFIG_READER_H_
#define _CONFIG_READER_H_

#include "pch.h"
#include "shared.h"
#include "tinyxml2.h"

using namespace tinyxml2;

class Config_Reader
{
    public:
        Config_Reader();
        bool LoadConfiguarionFile();
    private:
        bool IsValidFormat(string);
    protected:
        XMLDocument XMLdoc;
};

#endif
