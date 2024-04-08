#!/bin/bash
REV_HASH=`git describe --match init --dirty=+ --abbrev=12`
set -- "$REV_HASH"
IFS='-'; declare -a Array=($*)
REVISION=${Array[1]}
HASH=${Array[2]:1} 
DATE=`git show -s --format=%ci`

echo "#ifndef REVISION_H" > revision.h
echo "#define REVISION_H" >> revision.h
echo "    #define REVISION \"$REVISION\"" >> revision.h
echo "    #define HASH \"$HASH\"" >> revision.h
echo "    #define DATE \"$DATE\"" >> revision.h
echo "#endif // REVISION_H" >> revision.h
