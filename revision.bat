@echo off
git describe --match init --dirty=+ --abbrev=16 > temp.bak
FOR /F "tokens=2 delims=-" %%A IN (temp.bak) DO SET REVISION=%%A
FOR /F "tokens=3 delims=-" %%A IN (temp.bak) DO SET HASH=%%A
git show -s --format=%%ci > temp.bak
FOR /F "tokens=*" %%A IN (temp.bak) DO SET DATE=%%A
del temp.bak
echo #ifndef __REVISION_H__ > ..\revision.h
echo #define __REVISION_H__ >> ..\revision.h
echo     #define _REVISION "%REVISION%" >> ..\revision.h
echo     #define _HASH "%HASH:~1%" >> ..\revision.h
echo     #define _DATE "%DATE%" >> ..\revision.h
echo #endif // __REVISION_H__ >> ..\revision.h