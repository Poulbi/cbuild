#!/bin/sh

set -eu

ThisDir="$(dirname "$(readlink -f "$0")")"
cd "$ThisDir"

mkdir -p ../build > /dev/null 2>&1
cshell="../build/cshell"

if [ ! -x "$cshell" ] 
then
echo 
 clang -Wno-write-strings -g -o "$cshell" cshell.cpp
fi

../build/cshell
