<?php

namespace Parser;

const WOW_PARSER_LOG_OUTPUT = "wowparser4.log";

class Log
{
    private static ?Log $logClass = null;
    private $logFile;
    private bool $logFileIsOpen;
    public function __construct()
    {
        $this->logFile = fopen(WOW_PARSER_LOG_OUTPUT, "w+");
        $this->logFileIsOpen = $this->logFile !== false;
    }
    public function __destruct()
    {
        if ($this->logFileIsOpen)
            fclose($this->logFile);
    }
    public function append(string $text): void
    {
        if (!$this->logFileIsOpen)
            return;

       fwrite($this->logFile, $text);
    }
    protected static function log(String $str, bool $time, bool $print): void
    {
        if (!(Log::$logClass instanceof Log))
            Log::$logClass = new Log();

        if ($time) {
            $currentTime = date("Y-m-d H:i:s");
            $formatted_log = $currentTime . " " . $str;
        }
        else
            $formatted_log = $str;

        if ($print)
            echo $formatted_log;

        Log::$logClass->append($formatted_log);
    }
    public static function WriteLogAndPrint(...$fmt_args): void
    {
        Log::log(sprintf(...$fmt_args), true, true);
    }
    public static function WriteLogNoTimeAndPrint(...$fmt_args): void
    {
        Log::log(sprintf(...$fmt_args), false, true);
    }
    public static function WriteLog(...$fmt_args): void
    {
        Log::log(sprintf(...$fmt_args), true, false);
    }
    public static function WriteLogNoTime(...$fmt_args): void
    {
        Log::log(sprintf(...$fmt_args), false, false);
    }
}