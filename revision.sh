#!/bin/bash
REV_HASH=`git describe --match init --dirty=+ --abbrev=12`
set -- "$REV_HASH"
IFS='-'; declare -a Array=($*)
REVISION=${Array[1]}
HASH=${Array[2]:1}
DATE=`git show -s --format=%ci`
OUTPATH="$(dirname "$(readlink -f "$0")")"

echo "#ifndef REVISION_H" > $OUTPATH/revision.h
echo "#define REVISION_H" >> $OUTPATH/revision.h
echo "    #define REVISION \"$REVISION\"" >> $OUTPATH/revision.h
echo "    #define HASH \"$HASH\"" >> $OUTPATH/revision.h
echo "    #define DATE \"$DATE\"" >> $OUTPATH/revision.h
echo "#endif // REVISION_H" >> $OUTPATH/revision.h
