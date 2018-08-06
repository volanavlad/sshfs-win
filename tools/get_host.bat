:: SSHFS
:: Get host
:: 08/02/2018, San
::
@echo off

set HOST=10.0.0.10
set CLUSTER=cluster

:: pick random node between 01 and 32
:: set /A N=%RANDOM% * 32 / 32768 + 1
:: set N=0%N%
:: set N=%N:~-2%
:: set HOST=%CLUSTER%%N%

echo %HOST%