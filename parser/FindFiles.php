<?php

namespace Parser;

class FindFiles
{
    private static array $fileToFindClass = [];
    public static function FileToFind(string $directory,
                                      string $filename,
                                      string $structure,
                                      bool $recursive,
                                      string $fileExt,
                                      array $outFormats,
                                      int $xmlFileID = 0): void
    {
        // Just for testing
        self::$fileToFindClass[] = [$directory, $filename, $structure, $recursive, $fileExt, $outFormats, $xmlFileID];

        // TODO
    }
    public static function ListEmpty(): bool
    {
        return empty(self::$fileToFindClass);
    }
    public static function PrintAllFileNamesByFileType(): void
    {
        // Just for testing
        print_r(self::$fileToFindClass);

        // TODO
    }
}