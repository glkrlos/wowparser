#ifndef _PCH_H_
#define _PCH_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <map>
#include <fstream>
#include <vector>
#include <sstream>
#include <memory>
#include <utility>

#define _CODENAME "Zitacuaro"
#define _VERSION "3.1"

#ifdef __linux__
    #include "linux/getch.h"
    #define _OS "Linux"

    #ifdef __x86_64__
        #define _ARQUITECTURA "AMD64"
    #else
        #define _ARQUITECTURA "i386"
    #endif
#elif defined _WIN32 || defined _WIN64
    //#pragma warning(disable: 4996 4267)
    #include <conio.h>
    #define _OS "Windows"

    #ifdef _WIN64
        #define _ARQUITECTURA "x64"
    #else
        #define _ARQUITECTURA "x86"
    #endif
#endif

using namespace std;

#include "log.h"

#endif
