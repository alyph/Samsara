@echo off

call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" x64     
set compilerflags=/Od /Zi /MD
set linkerflags=
pushd %~dp0
cl.exe %compilerflags% @build/resp_msvc /link %linkerflags%
popd


