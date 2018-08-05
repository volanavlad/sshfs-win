:: SSHFS
:: Setup ssh keys for authentication
:: 07/31/2018, sganis
:: Dependencies: ssh, ssh-keygen,
 
@echo off
 
:: check for parameters
if [%1]==[] goto usage
 
set HOST=%1
 
set HOME=%USERPROFILE%
mkdir %HOME%\.ssh
 
:: generate ssh keys if not present already
if not exist %HOME%\.ssh\id_rsa (
	echo Generating ssh key pair...
	echo -n 'y/n' | ssh-keygen -f %HOME%\.ssh\id_rsa -q -N ""
) else (
	echo Private key already exists
	echo Generating public key...
	ssh-keygen -f %HOME%\.ssh\id_rsa -y > %HOME%\.ssh\id_rsa.pub             
)
:: read public key without end of line, save it to remote server
:: ssh-copy-id %HOST%
echo Saving public key to %HOST%...
more %USERPROFILE%\.ssh\id_rsa.pub ^
	| ssh %HOST% -o StrictHostKeyChecking=no "cat >> .ssh/authorized_keys; dos2unix .ssh/authorized_keys"
echo Done.
goto :eof
 
:usage
echo usage: %0 ^<ssh-server^>
exit /B 1