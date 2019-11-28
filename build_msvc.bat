@echo off

call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" x64     
set compilerflags=/Od /Zi /MD /GR- /MP /utf-8 /std:c++latest
set linkerflags=
pushd %~dp0
set "buildDir=build\win64\"
set "binDir=bin\win64\"
if not exist %buildDir% (md "%buildDir%")
if not exist %binDir% (md "%binDir%")

cl.exe %compilerflags% /Fo"%buildDir%\" /Fd"%buildDir%\" @tools/resp_msvc /link %linkerflags% /OUT:"%binDir%samsara.exe"
popd


