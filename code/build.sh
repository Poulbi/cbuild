#!/bin/sh

set -eu

ScriptDirectory="$(dirname "$(readlink -f "$0")")"
cd "$ScriptDirectory"

mkdir -p ../build
cd ../build

Output="./cbuild"

if [ ! -x "$Output" ] 
then
 cc -DOS_LINUX=1 -Wno-write-strings -g -o "$Output" ../code/example.c
fi

"$Output"
