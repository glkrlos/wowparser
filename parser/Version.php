<?php

const _VERSION = "4.0";
const _CODENAME = "Huamantla";
const _OS = "PHP";
const _ARQUITECTURA = "8.2";

$gitdata = explode("-", exec("git describe --match init --dirty=+ --abbrev=12"));
define("_REVISION", $gitdata[1]);
define("_HASH", $gitdata[2]);
define("_DATE", exec("git show -s --format=%ci"));
