:: SSHFS 
:: Installer
:: 08/05/2018, sganis

@echo off
setlocal

:: this script directory
set DIR=%~dp0
set DIR=%DIR:~0,-1%

set winfsp=
set winfsp=
for /f "tokens=5" %%i in ('dir %DIR% ^| findstr /r "winfsp.*.msi"') do set winfsp=%%i
for /f "tokens=5" %%i in ('dir %DIR% ^| findstr /r "sshfs.*x64.msi"') do set sshfs=%%i

echo installing %winfsp%...
msiexec /i %winfsp%

echo installing %sshfs%...
msiexec /i %sshfs%

echo installing tools...
set TOOLS="C:\Program Files\SSHFS-Win\tools"
mkdir %TOOLS%
xcopy /i %DIR% %TOOLS%

