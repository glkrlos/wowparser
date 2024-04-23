OREV := $(shell ./revision.sh)
all:
#	g++ -m32 -Os -s -Wall -msse2 -mfpmath=sse -std=c++20 -static -static-libgcc -static-libstdc++ src/findfiles.cpp src/log.cpp src/main.cpp src/md5.cpp src/module_config_reader.cpp src/module_csv_reader.cpp src/module_csv_writer.cpp src/module_dbc_writer.cpp src/module_parser.cpp src/module_sql_writer.cpp src/pch.cpp src/ProgressBar.cpp src/tinyxml2.cpp -o bin/wowparser3_i386
	g++ -m64 -Wall -std=c++20 src/findfiles.cpp src/log.cpp src/main.cpp src/md5.cpp src/module_config_reader.cpp src/module_csv_reader.cpp src/module_csv_writer.cpp src/module_dbc_writer.cpp src/module_parser.cpp src/module_sql_writer.cpp src/pch.cpp src/ProgressBar.cpp lib/tinyxml2/tinyxml2.cpp -o bin/wowparser4_amd64
#	i686-w64-mingw32-g++-win32 -Os -s -Wall -Wno-unused-function -msse2 -mfpmath=sse -std=c++20 -static -static-libgcc -static-libstdc++ src/findfiles.cpp src/log.cpp src/main.cpp src/md5.cpp src/module_config_reader.cpp src/module_csv_reader.cpp src/module_csv_writer.cpp src/module_dbc_writer.cpp src/module_parser.cpp src/module_sql_writer.cpp src/pch.cpp src/ProgressBar.cpp src/tinyxml2.cpp -o bin/wowparser3_x86.exe
#	x86_64-w64-mingw32-g++ -Os -s -Wall -Wno-unused-function -msse2 -mfpmath=sse -std=c++20 -static -static-libgcc -static-libstdc++ src/findfiles.cpp src/log.cpp src/main.cpp src/md5.cpp src/module_config_reader.cpp src/module_csv_reader.cpp src/module_csv_writer.cpp src/module_dbc_writer.cpp src/module_parser.cpp src/module_sql_writer.cpp src/pch.cpp src/ProgressBar.cpp src/tinyxml2.cpp -o bin/wowparser3_x64.exe
#	rm revision.h
