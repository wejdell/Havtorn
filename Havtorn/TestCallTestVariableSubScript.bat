@echo off


echo calling TestVariableSubScript.bat
call TestVariableSubScript.bat
echo.
echo reading testVariable after calling .bat is %testVariable%
PAUSE

echo.
echo calling MethodTest
call :MethodTest param1 what
echo.
echo after MethodTest: methodVariable is %methodVariable%
PAUSE

:MethodTest
echo this is a method test
echo setting a methodVariable
echo 2 params are %~1 and %~2 
set methodVariable="Method says hello"
goto:eof