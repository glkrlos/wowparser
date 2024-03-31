<?php

namespace Parser;

use DOMDocument;

class Config
{
    private function IsValidFormat(string $structure): bool
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

        // Prevenir mostrar mensajes de error
        libxml_use_internal_errors(true);

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
            Log::WriteLogNoTimeAndPrint("Failed: Syntax errors. Details in log.\n");

            foreach (libxml_get_errors() as $error) {
                $errorLevel = match($error->level) {
                    LIBXML_ERR_WARNING => "Warning",
                    LIBXML_ERR_ERROR => "Error",
                    LIBXML_ERR_FATAL => "Fatal",
                    default => null
                };

                // No deberia pasar nunca dado que aqui siempre debe haber errores
                if (is_null($errorLevel))
                    continue;

                Log::WriteLog($errorLevel . " error -> ");
                Log::WriteLogNoTime("Code: %d, Line: %d, Column: %d [%s]\n",
                    $error->code,
                    $error->line,
                    $error->column,
                    str_replace(array("\r", "\n"), '', $error->message));
            }

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

        $fileID = 0;
        foreach ($file_elements as $currentElementFile)
        {
            $attrib = array_fill_keys(['extension', 'name', 'recursive', 'directory', 'format', 'ToCSV', 'ToDBC', 'ToSQL'], null);

            $attributes = $currentElementFile->attributes;
            foreach ($attributes as $attribute)
            {
                if (!array_key_exists($attribute->nodeName, $attrib))
                    continue;

                $attrib[$attribute->nodeName] = $attribute->nodeValue;
            }

            // Sumamos uno al contador de <file> por que desde aqui empezamos a hacer las comprobaciones
            $fileID++;

            $FileExtension = !is_null($attrib['extension']) ? $attrib['extension'] : "";
            $FileExtensionIsSet = !empty($FileExtension);

            $FileName = !is_null($attrib['name']) ? $attrib['name'] : "";
            $Name = !empty($FileName);

            // Si no hay nombre continuamos
            if (!$Name && !$FileExtensionIsSet)
            {
                Log::WriteLog("\tWARNING: name attribute can't be empty in configuration file. Ignoring element number '%u'\n", $fileID);
                continue;
            }

            $isRecursive = false;
            // si el valor de recursive no esta establecido o es un valor incorrecto entonces ponemos que recursive is not set
            $RecursiveAttributeIsSet = is_bool($attrib['recursive']) && $attrib['recursive'];

            $_directoryName = !is_null($attrib['directory']) ? $attrib['directory'] : "";
            $DirectoryName = empty($_directoryName) ? "." : $_directoryName;

            if (!$RecursiveAttributeIsSet && $FileExtensionIsSet)
                $isRecursive = true;

            $FileFormat = !is_null($attrib['format']) ? $attrib['format'] : "";

            if (!$FileExtensionIsSet && !$this->IsValidFormat($FileFormat))
            {
                Log::WriteLogAndPrint("\t WARNING: For file name '%s' contains an invalid character in format attribute. Ignoring element '%u'\n", $FileName, $fileID);
                continue;
            }

            // Quizas mejor usar filter_var($var, FILTER_VALIDATE_BOOLEAN, FILTER_NULL_ON_FAILURE)) ???
            $outFormats = [
                'ToCSV' => $this->ValidateBool($attrib['ToCSV']),
                'ToDBC' => $this->ValidateBool($attrib['ToDBC']),
                'ToSQL' => $this->ValidateBool($attrib['ToSQL'])
            ];

            FindFiles::FileToFind($DirectoryName, $FileName, $FileFormat, $isRecursive, $FileExtensionIsSet ? $FileExtension : "", $outFormats, $fileID);
        }

        Log::WriteLog("-----> All OK after checking XML attributes of files to parse.\n");
        return true;
    }
    private function ValidateBool($var): bool
    {
        return !is_null($var) && (preg_match('/^(true)$/i', $var));
    }
}