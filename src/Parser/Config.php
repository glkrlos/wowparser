<?php

namespace Parser;

use DOMDocument;
use Helpers\Config\Utility;

class Config
{
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

        if (!Utility::CheckXMLSyntax($xmlfile))
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

                Log::WriteLog("%s -> Code: %d, Line: %d, Column: %d [%s]\n",
                    $errorLevel,
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
            $attributesList = array_fill_keys(
                ['extension', 'name', 'recursive', 'directory', 'format', 'ToCSV', 'ToDBC', 'ToSQL'],
                ['IsSet' => false, 'Value' => '', 'IsEmpty' => true]
            );

            foreach ($currentElementFile->attributes as $attribute)
            {
                $nodeName = $attribute->nodeName;
                if (!array_key_exists($nodeName, $attributesList))
                    continue;

                $attributesList[$nodeName]['IsSet'] = true;
                $attributesList[$nodeName]['Value'] = empty($attribute->nodeValue) ? "" : $attribute->nodeValue;
                $attributesList[$nodeName]['IsEmpty'] = empty($attribute->nodeValue) ?? true;
            }

            // Sumamos uno al contador de <file> por que desde aqui empezamos a hacer las comprobaciones
            $fileID++;

            $FileExtension = $attributesList['extension']['Value'];
            $FileExtensionIsSet = $attributesList['extension']['IsSet'];

            $FileName = $attributesList['name']['Value'];
            $Name = !$attributesList['name']['IsEmpty'];

            // Si no hay nombre continuamos
            if (!$Name && !$FileExtensionIsSet)
            {
                Log::WriteLog("\tWARNING: name attribute can't be empty in configuration file. Ignoring element number '%u'\n", $fileID);
                continue;
            }

            $isRecursive = false;
            // si el valor de recursive no esta establecido o es un valor incorrecto entonces ponemos que recursive is not set
            $RecursiveAttributeIsSet = is_bool($attributesList['recursive']['Value']) && $attributesList['recursive']['Value'];

            $_directoryName = $attributesList['directory']['Value'];
            $DirectoryName = $attributesList['directory']['IsEmpty'] ? "." : $_directoryName;

            if (!$RecursiveAttributeIsSet && $FileExtensionIsSet)
                $isRecursive = true;

            $FileFormat = $attributesList['format']['Value'];

            if (!$FileExtensionIsSet && !Utility::IsValidFormat($FileFormat))
            {
                Log::WriteLogAndPrint("\t WARNING: For file name '%s' contains an invalid character in format attribute. Ignoring element '%u'\n", $FileName, $fileID);
                continue;
            }

            // Quizas mejor usar filter_var($var, FILTER_VALIDATE_BOOLEAN, FILTER_NULL_ON_FAILURE)) ???
            $outFormats = [
                'ToCSV' => Utility::ValidateBool($attributesList['ToCSV']['Value']),
                'ToDBC' => Utility::ValidateBool($attributesList['ToDBC']['Value']),
                'ToSQL' => Utility::ValidateBool($attributesList['ToSQL']['Value'])
            ];

            FindFiles::FileToFind($DirectoryName, $FileName, $FileFormat, $isRecursive, $FileExtensionIsSet ? $FileExtension : "", $outFormats, $fileID);
        }

        Log::WriteLog("-----> All OK after checking XML attributes of files to parse.\n");
        return true;
    }
}