:: SSHFS 
:: Rebase dlls
::
@echo off
setlocal
set DIR=%~dp0
set DIR=%DIR:~0,-1%
set BIN=%1
set "PWD=%cd%"

cd %BIN%
echo current memory map:
%DIR%\rebase\rebase.exe -i *.dll

echo rebasing...
%DIR%\rebase\rebase.exe -b 0x400000000 *.dll

echo current memory map after rebasing:
%DIR%\rebase\rebase.exe -i *.dll

::for /f %%i in ('dir /b %BIN%\*.dll ^| findstr /v cygwin1.dll') do (
cd %PWD%

echo done.
