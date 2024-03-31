<?php

const _VERSION = "4.0";
const _CODENAME = "Huamantla";
const _TARGET = "PHP";
const _ARQUITECTURE = "8.2";
const _CONFIG_FILENAME = "../bin/wowparser4.xml";
const _LOG_FILENAME = "../bin/wowparser4.log";
const _XML_ROOT_NAME = "WoWParser4";

$gitdata = explode("-", exec("git describe --match init --dirty=+ --abbrev=12"));
define("_REVISION", $gitdata[1]);
define("_HASH", $gitdata[2]);
define("_DATE", exec("git show -s --format=%ci"));
