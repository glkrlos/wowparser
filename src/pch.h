#ifndef _PCH_H_
#define _PCH_H_

#ifdef _WIN32
    #pragma warning(disable: 4996 4267)
    #include <conio.h>
    #define _OS "Windows"
#else
    #include "linux/getch.h"
    #define _OS "Linux  "
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <map>
#include <fstream>
#include <vector>
#include <sstream>

using namespace std;

#endif
