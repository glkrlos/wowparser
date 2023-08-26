#!/bin/bash
REV_HASH=`git describe --match init --dirty=+ --abbrev=12`
set -- "$REV_HASH"
IFS='-'; declare -a Array=($*)
REVISION=${Array[1]}
HASH=${Array[2]:1}
DATE=`git show -s --format=%ci`

echo "#[allow(dead_code)]pub fn version() -> String { \"4.0\".to_string() }" > src/staticdata.rs
echo "#[allow(dead_code)]pub fn codename() -> String { \"Huamantla\".to_string() }" >> src/staticdata.rs
echo "#[allow(dead_code)]pub fn revision() -> String { \"$REVISION\".to_string() }" >> src/staticdata.rs
echo "#[allow(dead_code)]pub fn hash() -> String { \"$HASH\".to_string() }" >> src/staticdata.rs
echo "#[allow(dead_code)]pub fn date() -> String { \"$DATE\".to_string() }" >> src/staticdata.rs
echo "#[allow(dead_code)]pub fn os() -> String {
let os_text =
if cfg!(target_os = \"linux\")
{ \"Linux\" }
else if cfg!(target_os = \"windows\")
{ \"Windows\" }
else
{ \"Unknown OS\" };

os_text.to_string()
}" >> src/staticdata.rs
echo "#[allow(dead_code)]pub fn arch() -> String {
let arch_text =
if cfg!(target_arch = \"x86\")
{
    if cfg!(target_os = \"linux\")
    { \"i386\" }
    else
    { \"x86\" }
}
else if cfg!(target_arch = \"x86_64\")
{
    if cfg!(target_os = \"linux\")
    { \"AMD64\" }
    else
    { \"x64\" }
}
else
{ \"Unknown Arch\" };

arch_text.to_string()
}" >> src/staticdata.rs
