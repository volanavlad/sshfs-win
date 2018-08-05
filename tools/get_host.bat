:: SSHFS
:: Get host
:: 08/02/2018, San
::
@echo off

set HOST=192.168.56.1
set CLUSTER=vlcc

:: pick random node between 01 and 32
:: set /A N=%RANDOM% * 32 / 32768 + 1
:: set N=0%N%
:: set N=%N:~-2%
:: set HOST=%CLUSTER%%N%

echo %HOST%