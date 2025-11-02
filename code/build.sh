#!/bin/sh

set -eu

cd "$(dirname "$(readlink -f "$0")")"

mkdir -p ../build
cshell="../build/cshell"

if [ ! -x "$cshell" ] 
then
 clang -Wno-write-strings -g -o "$cshell" cshell.cpp
fi

$cshell norebuild rebuild