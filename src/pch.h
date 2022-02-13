#ifndef _PCH_H_
#define _PCH_H_

#ifdef __linux__
    #include <dirent.h>
    #include "linux/getch.h"
    #define _OS "Linux  "
#else // __WIN32 || __WIN64
    #include "win32/dirent.h"
    #include <conio.h>
    #define _OS "Windows"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <map>
#include <fstream>
#include <vector>
#include <sstream>
#include <memory>

#include "log.h"

using namespace std;

#endif
