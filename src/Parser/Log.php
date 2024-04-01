<?php

namespace Parser;

class Log
{
    private static ?Log $logClass = null;
    private $logFile;
    private bool $logFileIsOpen;
    public function __construct()
    {
        $this->logFile = fopen(_LOG_FILENAME, "w+");
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
    protected static function add(string $str, bool $time, bool $print): void
    {
        if (!(Log::$logClass instanceof Log))
            Log::$logClass = new Log();

        $formatted_log = $str;

        if ($time) {
            $currentTime = date("Y-m-d H:i:s");
            $formatted_log = $currentTime . " " . $formatted_log;
        }

        if ($print)
            echo $formatted_log;

        Log::$logClass->append($formatted_log);
    }
    public static function WriteLogAndPrint(...$fmt_args): void
    {
        Log::add(sprintf(...$fmt_args), true, true);
    }
    public static function WriteLogNoTimeAndPrint(...$fmt_args): void
    {
        Log::add(sprintf(...$fmt_args), false, true);
    }
    public static function WriteLog(...$fmt_args): void
    {
        Log::add(sprintf(...$fmt_args), true, false);
    }
    public static function WriteLogNoTime(...$fmt_args): void
    {
        Log::add(sprintf(...$fmt_args), false, false);
    }
}