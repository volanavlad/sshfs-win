:: SSHFS 
:: Uninstaller
:: 08/05/2018, sganis

@echo off
setlocal

:: this script directory
set DIR=%~dp0
set DIR=%DIR:~0,-1%

echo uninstalling tools...
set TOOLS="C:\Program Files\SSHFS-Win\tools"
rmdir /s /q %TOOLS%

set winfsp=
set winfsp=
for /f "tokens=5" %%i in ('dir %DIR% ^| findstr /r "winfsp.*.msi"') do set winfsp=%%i
for /f "tokens=5" %%i in ('dir %DIR% ^| findstr /r "sshfs.*x64.msi"') do set sshfs=%%i

echo uninstalling %winfsp%...
msiexec /x %winfsp%

echo uninstalling %sshfs%...
msiexec /x %sshfs%


