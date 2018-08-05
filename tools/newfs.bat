:: SSHFS
:: Create a new sshfs name using winfsp fsreg.bat
:: so users can mount \\fsname\host
:: 08/04/2018, sganis

@echo off
setlocal

if [%1]==[] goto :usage

set WINFSP="C:\Program Files (x86)\WinFsp\bin\fsreg.bat"

if "%2"=="-u" (
	%WINFSP% -u %1
) else (
	%WINFSP% %1 "C:\Program Files\SSHFS-Win\bin\sshfs-win.exe" "%%1 %%2 %%U" "D:P(A;;RPWPLC;;;WD)"
)
goto :eof

:usage
echo usage: %0 ^<name^> [-u]
exit /B 1