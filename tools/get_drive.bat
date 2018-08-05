:: SSHFS
:: Get available drive
:: 08/01/2018, sganis

@echo off
setlocal
setlocal enabledelayedexpansion

:: get list of mounted drives
set count=0
for /f "tokens=1" %%i in ('net use ^|find ":"') do (
	set drives[!count!]=%%i
	set /a count+=1
)
set /a count-=1

:: find first available letter strating from Z
set letter="No drive available"
for %%d in (Z: Y: X: W: V: U: T: S: R: Q: P: O: N: M: L: K: J: I: H: G: F:) do (
	set busy=0
	for /l %%n in (0, 1, %count%) do (
		if !drives[%%n]!==%%d set busy=1
	)
	if !busy!==0 (
		set letter=%%d
		goto :next
	)
)	
:next
echo %letter%

