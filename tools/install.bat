:: SSHFS 
:: Installer
:: 08/05/2018, sganis

::@echo off
setlocal

:: this script directory
set DIR=%~dp0
set DIR=%DIR:~0,-1%

set winfsp=
set winfsp=
for /f "tokens=5" %%i in ('dir %DIR% ^| findstr /r "winfsp.*.msi"') do set winfsp=%%i
for /f "tokens=5" %%i in ('dir %DIR% ^| findstr /r "sshfs.*x64.msi"') do set sshfs=%%i

echo installing %winfsp%...
::msiexec /i %winfsp%

echo installing %sshfs%...
::msiexec /i %sshfs%

echo installing tools...
set SSHFS="C:\Program Files\SSHFS-Win"
mkdir %SSHFS%\tools
xcopy /i %DIR%\..\tools %SSHFS%\tools

:: set home directory
echo db_home: windows > %SSHFS%\etc\nsswitch.conf

:: Set user permissions
icacls %SSHFS%\tools /grant %USERNAME%:(oi)(ci)f /t

pause
