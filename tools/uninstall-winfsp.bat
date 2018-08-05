:: SSHFS 
:: Uninstall winsfp
:: 08/05/2018, sganis

@echo off
setlocal

:: this script directory
set DIR=%~dp0
set DIR=%DIR:~0,-1%

set winfsp=
for /f "tokens=5" %%i in ('dir %DIR% ^| findstr /r "winfsp.*.msi"') do set winfsp=%%i

echo stopping services...
taskkill /im sshfs.exe
taskkill /im sshfs-win.exe
taskkill /im ssh.exe

echo uninstalling %winfsp%...
msiexec /x %winfsp%


