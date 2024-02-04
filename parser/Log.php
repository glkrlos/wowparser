<?php

namespace Parser;

class Log
{
    protected static function log(String $str, bool $time): void
    {
        if ($time) {
            $currentTime = date("Y-m-d H:i:s");
            echo $currentTime . " " . $str;
            return;
        }

        echo $str;
    }
    public static function WriteLogAndPrint(...$va_arguments): void
    {
        $formated = sprintf(...$va_arguments);
        Log::log($formated, true);
    }
    public static function WriteLog(...$va_arguments): void
    {
        $formated = sprintf(...$va_arguments);
        Log::log($formated, true);
    }
}