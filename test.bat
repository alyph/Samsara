@echo off

pushd %~dp0\bin\win64

echo ********** BEGIN TESTS **********


call allocation_test.exe
if %ERRORLEVEL% NEQ 0 goto TEST_FAILED

call string_test.exe
if %ERRORLEVEL% NEQ 0 goto TEST_FAILED

goto TEST_SUCCESS


:TEST_FAILED
popd
echo ERROR: %ERRORLEVEL%
echo ********** TEST FAILED **********
exit /b 1

:TEST_SUCCESS
popd
echo ********** TEST SUCCESS **********



