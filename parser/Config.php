<?php

namespace Parser;

use DOMDocument;

class Config
{
    private function CheckXMLSyntax($xmlData): bool
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
    public function Load(): bool
    {
        Log::WriteLogAndPrint("-----> Loading Configuration file... ");

        if (!file_exists(_CONFIG_FILENAME))
        {
            Log::WriteLogNoTimeAndPrint("Unable to locate configuration file.\n");
            return false;
        }

        $dom = new DOMDocument;

        if (!is_readable(_CONFIG_FILENAME))
        {
            Log::WriteLogNoTimeAndPrint("Unable to read configuration file.\n");
            return false;
        }

        $xmlfile = file_get_contents(_CONFIG_FILENAME);
        if ($xmlfile === false)
        {
            Log::WriteLogNoTimeAndPrint("Unable to open configuration file.\n");
            return false;
        }

        if (!$this->CheckXMLSyntax($xmlfile))
        {
            Log::WriteLogNoTimeAndPrint("Failed: Syntax errors.\n");
            return false;
        }

        if ($dom->loadXML($xmlfile) === false)
        {
            Log::WriteLogNoTimeAndPrint("Failed: Unexpected error.\n");
            return false;
        }

        if ($dom->documentElement->nodeName !== _XML_ROOT_NAME)
        {
            Log::WriteLogNoTimeAndPrint("Failed: Invalid XML file.\n");
            return false;
        }

        $file_elements = $dom->getElementsByTagName('file');
        $num_files = $file_elements->length;

        if (!$num_files)
        {
            Log::WriteLogNoTimeAndPrint("Failed: No files to parse.\n");
            return false;
        }

        Log::WriteLogNoTimeAndPrint("OK\n");

        Log::WriteLog("\n");
        Log::WriteLog("-----> Checking XML attributes of files to parse...\n");

        // TODO: Implementar la lectura de cada elemento <file>

        Log::WriteLog("-----> All OK after checking XML attributes of files to parse.\n");

        return true;
    }
}