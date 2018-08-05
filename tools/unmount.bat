:: SSHFS
:: Unmount drive
:: 08/01/2018, sganis
:: usage: unmount <drive>

@echo off
setlocal

if [%1]==[] goto :usage
set DRIVE=%1
set LETTER=%DRIVE:~0,-1%

:: calling net use /del 3 times
set REGPATH=
set D=
set MOUNT=

for /f "tokens=3" %%i in ('reg query HKCU\Network\%LETTER% /v RemotePath 2^>nul') do (
	set REGPATH=%%i
)
for /f "tokens=1-2" %%i in ('net use ^| findstr /i %DRIVE%') do (
	set D=%%i
	set MOUNT=%%j
)
if /i "%MOUNT%"=="%DRIVE%" (
	for /f "tokens=1-3" %%i in ('net use ^| findstr /i %DRIVE%') do (
		set D=%%j
		set MOUNT=%%k
	)
)

if not "%REGPATH%" == "" (
	echo unmounting registry path %REGPATH%...
	net use %REGPATH% /del 
)
if not "%D%" == "" (
	echo unmounting drive %D%...
	net use %D% /del 
)
if not "%MOUNT%" == "" (
	echo unmounting mounted path %MOUNT%...
	net use %MOUNT% /del 2>nul
)

:: kill processes
::tasklist | findstr /r "^ssh[fs]*[win-]*.exe"
::taskkill /im sshfs.exe /f
::taskkill /im sshfs-win.exe /f
::taskkill /im ssh.exe /f

goto :eof

:usage
echo usage: %0 ^<drive^>
exit /B 1