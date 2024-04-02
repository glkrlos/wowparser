<?php

namespace Helpers\Config;

class Utility
{
    public static function CheckXMLSyntax($xmlData): bool
    {
        if ( empty($xmlData) )
            return false;

        $xmlcontent = $xmlData;
        if( substr($xmlcontent, 0,3) == pack("CCC",0xef,0xbb,0xbf) )
            $xmlcontent = substr($xmlcontent, 3);

        if (!mb_check_encoding($xmlcontent,"utf-8"))
            $xmlcontent = mb_convert_encoding($xmlcontent, 'UTF-8', 'ISO-8859-1');

        $newxmlcontent = mb_convert_encoding($xmlcontent, 'UTF-8', 'ISO-8859-1');

        if (mb_check_encoding($newxmlcontent,"utf-8") && $newxmlcontent != $xmlcontent)
            $xmlcontent = $newxmlcontent;

        if (stripos($xmlcontent, '<!DOCTYPE html') !== false || stripos($xmlcontent, '<html') !== false)
            return false;

        if(simplexml_load_string($xmlcontent) === FALSE)
            return false;

        return true;
    }
    public static function IsValidFormat(string $structure): bool
    {
        /*
         * X -> unk byte
         * b -> byte
         * s -> string
         * f -> float
         * d -> int
         * n -> int
         * x -> unk int
         * i -> int
         * u -> unsigned int
         */
        return preg_match('/^[Xbsfdnxiu]+$/i', $structure);
    }
    public static function ValidateBool($var): bool
    {
        return preg_match('/^(true)$/i', $var);
    }
}