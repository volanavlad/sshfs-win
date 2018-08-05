:: SSHFS
:: Clean mounts
:: 08/01/2018, sganis

@echo off

set regkey=HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer\MountPoints2

set count=0
for /f "delims=" %%e in ('reg query %regkey% ^|findstr /c:"##sshfs"') do (
	echo deleting %%e
	reg delete %%e /f
	set /a count+=1
)

if %count%==0 (
	echo nothing to clean.
) else (
	echo %count% record/s cleaned.
)

::set mru="HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer\Map Network Drive MRU"
::HKEY_USERS\S-1-5-21-1824426863-1872128206-3493803034-1001\Network\y