@echo off

call C:\msvc\setup_x64.bat

REM Go to parent dir
cd %~dp0

IF NOT EXIST ..\build mkdir ..\build

pushd ..\build

set CommonCompilerFlags=-MTd -Gm- -nologo -GR- -EHa- -Oi -FC -Z7 -WX -W4 -wd4459 -wd4456 -wd4201 -wd4100 -wd4101 -wd4189 -wd4505 -wd4996 -wd4389 -wd4244 

cl %CommonCompilerFlags% -I..\.. -DOS_WINDOWS=1 ..\code\cbuild.c