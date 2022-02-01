DOREV := $(shell ./revision.sh)
all:
	g++ src/tinyxml2.cpp src/dbc_reader.cpp src/main.cpp -o bin/binaryreader
	i586-mingw32msvc-c++ --no-warnings src/tinyxml2.cpp src/dbc_reader.cpp src/main.cpp -o bin/binaryreader.exe
	rm revision.h
