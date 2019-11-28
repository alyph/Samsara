@echo off

pushd %~dp0

for /f "usebackq tokens=*" %%i in (`.\external\msvc\vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
	set InstallDir=%%i
)

if not exist "%InstallDir%\VC\Auxiliary\Build\vcvarsall.bat" (
	echo Error: invalid VS installation "%InstallDir%"
	exit /b 1
)

echo Building using VS installation: %InstallDir%
echo.

call "%InstallDir%\VC\Auxiliary\Build\vcvarsall.bat" x64

make %*
popd
