@echo off

call C:\msvc\setup_x64.bat

REM Go to parent dir
cd %~dp0

IF NOT EXIST ..\build mkdir ..\build

pushd ..\build

cl -FC -nologo -I..\.. -DOS_WINDOWS=1 ..\code\build.c