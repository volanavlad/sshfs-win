:: SSHFS
:: Get comand line arguments from sshfs process

WMIC path win32_process get Caption,Processid,Commandline | findstr ssh