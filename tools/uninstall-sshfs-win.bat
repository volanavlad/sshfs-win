:: SSHFS 
:: Uninstall sshfs-win
:: 08/05/2018, sganis

@echo off
setlocal

:: this script directory
set DIR=%~dp0
set DIR=%DIR:~0,-1%

echo uninstalling tools...
set TOOLS="C:\Program Files\SSHFS-Win\tools"
rmdir /s /q %TOOLS%

set sshfs=
for /f "tokens=5" %%i in ('dir %DIR% ^| findstr /r "sshfs.*x64.msi"') do set sshfs=%%i

echo stopping services...
taskkill /im sshfs.exe
taskkill /im sshfs-win.exe
taskkill /im ssh.exe

echo uninstalling %sshfs%...
msiexec /x %sshfs%


