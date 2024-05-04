#ifndef WOWPARSER_VERSION_H
#define WOWPARSER_VERSION_H

#define CODENAME "Huamantla"
#define VERSION "4.0"
#define CONFIG_FILENAME "wowparser4.xml"

#ifdef __linux__
#include "platform/linux/getch.h"
    #define OS "Linux"

    #ifdef __x86_64__
        #define ARQUITECTURA "AMD64"
    #else
        #define ARQUITECTURA "i386"
    #endif
#elif defined _WIN32 || defined _WIN64
//#pragma warning(disable: 4996 4267)
    #include <conio.h>
    #define OS "Windows"

    #ifdef _WIN64
        #define ARQUITECTURA "x64"
    #else
        #define ARQUITECTURA "x86"
    #endif
#endif

#endif //WOWPARSER_VERSION_H
