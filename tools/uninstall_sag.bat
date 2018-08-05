:: SSHFS 
:: Uninstaller for my SSHFS version
:: 08/05/2018, sganis

@echo off
setlocal

:: this script directory

set winfsp=
for /f "tokens=5" %%i in ('dir %DIR% ^| findstr /r "sshfs.*x64-SAG.msi"') do set sshfs=%%i

echo stopping services...
taskkill /im sshfs.exe
taskkill /im sshfs-win.exe
taskkill /im ssh.exe

echo uninstalling %sshfs%...
msiexec /x %sshfs%


