#!/bin/sh

set -eu

ScriptDirectory="$(dirname "$(readlink -f "$0")")"
cd "$ScriptDirectory"

mkdir -p ../build
cshell="../build/cshell"

if [ ! -x "$cshell" ] 
then
 cc -Wno-write-strings -g -o "$cshell" cshell.cpp
fi

$cshell
