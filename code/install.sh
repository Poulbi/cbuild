#!/bin/sh

CurrentDir="$(readlink -f ".")"

ScriptDir="$(dirname "$(readlink -f "$0")")"
cd "$ScriptDir"

File="build.h"

ln -sf "$(readlink -f "$File")" "$CurrentDir/$File" &&
	>&2 printf 'Installed.\n'
