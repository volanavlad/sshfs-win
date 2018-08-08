:: SSHFS
:: Mount linux filesystem in windows drive
:: Usage: mount.bat [host] [drive]
:: 07/31/2018, sganis

@echo off
setlocal

:: this script directory
set DIR=%~dp0
set DIR=%DIR:~0,-1%

:: default values
set DRIVE=Z:
set HOST=LINUX
set DRIVENAME=LINUX

if "%1"=="-h" goto :usage

:: find next available drive
if not "%~1"=="" (
	set DRIVE=%1
) else (
	for /f %%i in ('%DIR%\get_drive.bat') do set DRIVE=%%i
)
echo Drive: %DRIVE%

:: get host from parameter or get_host.bat program
if not "%~2"=="" (
	set HOST=%2
) else (
	for /f %%i in ('%DIR%\get_host.bat') do set HOST=%%i
)
echo Selected host: %HOST%

:: get info from environment variables
set USER=%USERNAME%
set HOME=%USERPROFILE%
set LOG1=%HOME%\.ssh\winfsp.log
set LOG2=%HOME%\.ssh\sshfs.log
::echo SSHFS Log file > %LOG%
set SSHFS=%ProgramFiles%\SSHFS-Win\bin
set PATH=%SSHFS%;%PATH%

:: create ssh config file
::set CONFIG=%HOME%/.ssh/sshfs.config
::if not exist %HOME%/.ssh/ md %HOME%/.ssh/
::echo IdentityFile %HOME%/.ssh/id_rsa >%CONFIG%
::echo UserKnownHostsFile %HOME%/.ssh/known_hosts >>%CONFIG%
::echo StrictHostKeyChecking no >%CONFIG%
::echo ServerAliveInterval 10 >>%CONFIG%

:: set drive name
echo Setting drive name as: %DRIVENAME%
set REGKEY=HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer\MountPoints2\##sshfs#%USER%@%HOST%
reg add %REGKEY% /v _LabelFromReg /d %DRIVENAME% /f

:: run sshfs with params
echo on
"%SSHFS%\sshfs.exe" %USER%@%HOST%:/ %DRIVE% -f -orellinks -oreconnect ^
    -ouid=-1,gid=-1,create_umask=0007 -oVolumePrefix=/sshfs/%USER%@%HOST% ^
    -ovolname=LINUX-%HOST% -oFileSystemName=SSHFS -oStrictHostKeyChecking=no ^
    -oServerAliveInterval=10 -oServerAliveCountMax=10000 ^
    -oFileInfoTimeout=10000 -oDirInfoTimeout=10000 -oVolumeInfoTimeout=10000 ^
    -ossh_command='ssh -vv'
	::-ossh_command='autossh -M 0 -vv' ^
    
goto :eof

:usage
echo usage: %0 [drive] [host] 
exit /B 1

:: redirect stderr and stdout to log file
:: >> %LOG2% 2>&1


::sshfs.exe %USER%@%HOST%:%MOUNTPATH% %DRIVE% ^
::             -o rellinks ^
::             -o reconnect ^
::             -o uid=-1,gid=-1,create_umask=0007 ^
::             -o VolumePrefix=/sshfs/%USER%@%HOST% ^
::             -o FileSystemName=SSHFS ^
::             -o debug -o sshfs_debug -o LOGLEVEL=DEBUG3 -o ssh_command="ssh -vv" ^
::             -o UserKnownHostsFile=%HOME%/.ssh/known_hosts ^
::             -o StrictHostKeyChecking=no ^
::             -f
