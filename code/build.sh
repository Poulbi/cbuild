#!/bin/sh

set -eu

ScriptDirectory="$(dirname "$(readlink -f "$0")")"
cd "$ScriptDirectory"

mkdir -p ../build
cd ../build

Output="./build"

if [ ! -x "$Output" ] 
then
 cc -DOS_LINUX=1 -Wno-write-strings -g -o "$Output" $(pwd)/../code/build.c
fi

"$Output"
