<?php

use Parser\Config;
use Parser\Log;

header("Content-Type: text/plain");

error_reporting(E_ALL);
ini_set('display_errors', 1);
ini_set('memory_limit', '2048M');
set_time_limit(0);

spl_autoload_register(function ($class) {
    $class = lcfirst(str_replace('\\', '/', $class)) . '.php';
    if (file_exists(__DIR__ . "/" . $class))
        include_once __DIR__ . "/" . $class;
});

function getch(): void
{
    system('stty -icanon');
    fread(STDIN, 1);
    system('stty icanon');
}

include_once "parser/Version.php";

function printHeader(): void
{
    Log::WriteLogAndPrint("WoWParser Version %s (%s) in Development State for %s %s (Revision: %s)\n", _VERSION, _CODENAME, _OS, _ARQUITECTURA, _REVISION);
    Log::WriteLogAndPrint("Hash: %s\tDate: %s\n", _HASH, _DATE);
    Log::WriteLogAndPrint("\n");
    Log::WriteLogAndPrint("Tool to Parse World of Warcraft files (DBC DB2 ADB WDB).\n");
    Log::WriteLogAndPrint("Copyright(c) 2024 Carlos Ramzuel - Tlaxcala, Mexico.\n");
    Log::WriteLogAndPrint("\n");

    Log::WriteLog("====================================LOG FILE START====================================\n");
}

function pass1LoadConfig() {
    $test = new Config();
}

function pass2PrintFilesToLog() {
}

function pass3CheckHeadersAndDataConsistency() {
}

function printEnd(): void
{
    Log::WriteLogAndPrint("-----> Finished\n");
    Log::WriteLog("=====================================LOG FILE END=====================================\n");
    printf("--Press any key to exit--\n");
    getch();
}

printHeader();
pass1LoadConfig();
pass2PrintFilesToLog();
pass3CheckHeadersAndDataConsistency();
printEnd();