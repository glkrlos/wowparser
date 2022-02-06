#ifndef _PCH_H_
#define _PCH_H_

#if (_MSC_VER == 1800)
#define _CRT_SECURE_NO_WARNINGS
#include <time.h>
#pragma warning(disable:4996)
#endif

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
#include <stdarg.h>
#include <string>
#include <map>
#include <fstream>
#include <vector>
#include <sstream>

using namespace std;

#endif
