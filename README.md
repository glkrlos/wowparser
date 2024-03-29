## WoWParser is a Tool to Parse World of Warcraft files (DBC ADB DB2 WDB) to a various file formats.

Copyright(c) 2024 - Carlos Ramzuel - Tlaxcala, Mexico

## WoWParser 4.0 (Huamantla) in Development State:
The project will be completely rebuilt in:
 - Rust language at [master branch](https://github.com/glkrlos/wowparser)
 - PHP language at [php branch](https://github.com/glkrlos/wowparser/tree/php)

New version for language C++ still pending for development.

## Versions up to 3.1 are in the C++ language
 - These versions are in branch [stable_cplus](https://github.com/glkrlos/wowparser/tree/stable_cplus)

## WoWParser 3.1 (Zitacuaro):
 * New Version 3.1
 * New: For each new version of parser now will have a codename
 * New: Enabled the parse of DB2 and WDB files. For WDB files requires maximum version 15595 to parse it. Just the parse of itemcache.wdb is disabled for now.
 * Now have 4 executables, two for Linux (i386 and AMD64) and two for Windows (x86 and x64), and is compiled under linux debian.
 * New: Implemented the option to parse each file type to CSV, DBC or SQL formats, the default output is to SQL files, but you can manually choice what output formats do you want.
 * Improvements: Added additional check at the moment of parsing CSV files to prevent some possible errors, and now requires to not have any empty line even if is the last.

Testing:
 * Support to verify converted files.
 * Support to connect to a SQL Server. For read and write parsed files.

Still in Development:
 * Creating support to predict field types in WDB files. (really hard to do that but i'm in progress)
 * Support to select name and order of columns in SQL file format.
 * Support to select the name and order of columns at the moment of parse files to a SQL Server.

## WoWParser 3.0:
 * New Version 3.0 Revision 192 has been Released.
 * Temporarily disabled the parse of DB2 and WDB files because this will need more tests.
 * Predicting DBC and ADB files are now 100% efficient for non byte packed files.
 * Temporarily disabled the creation of SQL files because an unknown error.
 * For now is realeased only for Windows, I'm still testing over different platforms, but i will end son.
 * Improved in CSV files how is handled the parse of special characters like \t \r or \n respectively.
          \t is replaced by [[[[t]]]], \n by {{{{n}}}} and \r by ||||r||||, this will prevent a CSV file to be wrong.
          This is a workaround to fix all the cases when the string have new line, line return, or tabulator because
          CSV files can't handle it even if you use tabs as separator. When the program reads the CVS files again,
          these especial characters will return to the original state at the moment of write to new file format.
 * I'm really sorry if i waited too long, but now i can activelly working on this proyect.
 * Now are two executables one for linux and the other for windows.
 * No longer compiled under Windows.
 * Now is compiled under Visual Studio Community Edition For Windows and under Debian for Linux
 * Now is created a log with detailed information of everything that happens.
 * Configuration file changed to XML format.
 * Now uses Version Control (Git).
 * Revision is used instead of Build.
 * Compiled sources now include HASH and DATE from repository to know what Revision is.
 * No longer has two windows executables, now is all in one.
 * Modules to read CSV, DBC, DB2, ADB files are totally redone. All the code are totally rewritten.
 * Reading CSV files now shows correct messages if expected data is wrong.
 * Predict field types now gets unsigned int fields.

## WoWParser 2.0:
Initial Build:
 * Version 2.0

Build 12:
 * Improved optimization at read binary files.

Build 73:
 * Corrected many problemas at parse CSV files.
 * Added many error messages to known the cause of problem at parsing file.
 * Added support to read DB2 files (after patched mode).
 * Finally predict field types working very good with float and string fields.
 * Removed support to read files via arguments in command line.
 * Implemented support to find and read files using recursive mode in the program directory.
 * Implemented support to read files with specific format (string, int, float and byte fields).
 * Added configuration file to read files and format.
 * Implemented in configuration find files using *.dbc for example.
 * Implemented creation of DBC files using recursive mode.

Build 81:
 * Fixed a bug where Predict System can't open some files because an incorrect error message of byte packed.
 * Improved Predict String Fields from ADB, DB2 and DBC files (Report any bugs if any with this improvement).
 * Improved Parse Strings Fields from CSV files, now display proper error messages in unexpected end of string or missing " at the end of string field.

Build 85:
 * Fixed a crash if the only one field is set (string field) at parse CSV files.
 * Fixed a problem if the first field is a string, the rest of the fields can cause an incorrect error messages.

Build 98 (Final 2.0 Version):
 * Improved code optimization to read in a better and safe way binary files.
 * Implemented support to read WDB files (Only in configuration file with the proper format).
 * Improved format in configuration file to read in one special way only itemcache.wdb to parse it correctly.

Known Bugs in version 2.0:
 * Reading values from CSV file for integer, float, and byte fields still no error message if you put an alphabetical character, normally in conversion to numeric value is cero '0', so beware.

## WoWParser 1.0:
 * Implement support to read ADB, DBC files.
 * Added Basic Support to predict integer, string and float field types (Predict float and string fields still not working good and byte fields not supported yet).
 * Added support to read files via arguments in command line.
 * Implemented Read CSV files.

## How To Compile:
 Compiled under Linux Debian 12
 1. apt-get update
 2. apt-get upgrade
 3. apt-get install curl build-essential

NOTE: The installation of Rust is on current user in their home directory

 4. curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
 5. source $HOME/.cargo/env
 6. Test rust: rustc --version

## License

[MIT License](LICENSE)

## Reporting issues

Issues can be reported [here](https://github.com/glkrlos/wowparser/issues).

## Submitting fixes or changes

Fixes or changes can be request [here](https://github.com/glkrlos/wowparser/pulls).

Before you submit a pull request, read file [CONTRIBUTING](CONTRIBUTING.md).
