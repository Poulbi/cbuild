#!/bin/sh

mkdir -p ../build/

ScriptDirectory="$(dirname "$(readlink -f "$0")")"
cd "$ScriptDirectory"

clang -Wno-write-strings -o ../build/cshell cshell.cpp
