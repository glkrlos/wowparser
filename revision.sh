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
echo "#endif // __REVISION_H__" >> revision.h
