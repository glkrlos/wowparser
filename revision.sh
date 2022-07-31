#!/bin/bash
REV_HASH=`git describe --match init --dirty=+ --abbrev=12`
set -- "$REV_HASH"
IFS='-'; declare -a Array=($*)
REVISION=${Array[1]}
HASH=${Array[2]:1} 
DATE=`git show -s --format=%ci`

echo "#ifndef __REVISION_H__" > revision.h
echo "#define __REVISION_H__" >> revision.h
echo "    #define _REVISION \"$REVISION\"" >> revision.h
echo "    #define _HASH \"$HASH\"" >> revision.h
echo "    #define _DATE \"$DATE\"" >> revision.h
echo "    #define LINE1 \"WoWParser Version 3.0 for %s (Revision: %s)\n\", _OS, _REVISION" >> revision.h
echo "    #define LINE2 \"Hash: %s\tDate: %s\n\", _HASH, _DATE" >> revision.h
echo "    #define LINE_NEW \"\n\"" >> revision.h
echo "    #define LINE3 \"Tool to Parse World of Warcraft files (DBC DB2 ADB WDB).\n\"" >> revision.h
echo "    #define LINE4 \"Copyright(c) 2022 Carlos Ramzuel - Tlaxcala, Mexico.\n\"" >> revision.h
echo "#endif // __REVISION_H__" >> revision.h
