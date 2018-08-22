# SSHFS for Windows

This is a fork of https://github.com/billziss-gh/sshfs-win. 

# TODO List

- SSH authentication
- Mount path simplification
- Tools for deployment/mount/unmount/clenaup
- Set drive name
- Add -o volname
- Read json config file
- Get host from config file
- Get user from environment variable
- Tunning ssh parameters for performance

# Development Notes

## Build WinFsp

1. Install Visual Studio 2015 Community, use the option "Tipical for Windows 10 development"
   (https://go.microsoft.com/fwlink/p/?LinkId=534599)

2. Install Windows SDK for Windows 10, version 1703
   (https://go.microsoft.com/fwlink/p/?LinkID=845298)

3. Install WDK for Windows 10, version 1703
   (https://go.microsoft.com/fwlink/p/?LinkID=845980)

4. Install Wix Toolset and VS 2015 extension
   (http://wixtoolset.org/releases)


## Build Sshfs-win

1. Install SSHFS-Win 3.2 BETA, WinFsp 2018.2 B2, WIX Toolset 3.11. Versions that I used in this test:
   
   SSHFS-Win 3.2 BETA, WinFsp 2018.2 B2, WIX Toolset 3.11.

2. Install cygwin
   
   Download from https://www.cygwin.com/setup-x86_64.exe. Run installer and follow default settings

3. Install development tools. Copy setup-x86_64.exe to `C:\cygwin64`, then from a cygwin terminal run:
   
   `$ /setup-x86_64.exe -q -B -P gcc-g++,make,automake,patch,vim,git,libglib2.0-devel,cygport,meson`

4. Install cygfuse by running:
   
   `$ /cygdrive/c/Program\ Files\ \(x86\)/WinFsp/opt/cygfuse/install.sh`

5. Clone the sshfs-win repository
   
   `$ git clone https://github.com/billziss-gh/sshfs-win.git`

6. Update the sshfs submodule. Go to the `sshfs-win` project directory, then run:
   
   `$ git submodule init`
   `$ git submodule update`

7. Run make in parallel mode 
   
   `$ make -j8`

8. An msi file installer will be saved in `.build/x64/dist` folder. It builds with some warnings that I coundn't get rid of, such as missing rst2man package.


## Test

```
set HOST=linux
:: replace \ by / in the path
:: setting HOME is not needed in the last version
set HOME=%USERPROFILE:\=/%
cd C:\Program Files\SSHFS-Win\bin
set PATH=C:\Program Files\SSHFS-Win\bin;%PATH%

sshfs.exe %USER%@%HOST%:/../..  X: -o rellinks -o reconnect -f ^
   -o VolumePrefix=/sshfs/%USER%@%HOST%/../.. -o volname=/sshfs/%USER%@%HOST% ^
   -o uid=-1,gid=-1,create_umask=0007 -o FileSystemName=SSHFS
```

## Set drive name

```
set DRIVENAME=Simulation
set REGKEY=HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer\MountPoints2\##sshfs#%USER%@%HOST%
reg add %REGKEY% /v _LabelFromReg /d %DRIVENAME% /f
```

## Debug sshfs

```
sshfs -o idmap=user -o debug -o sshfs_debug -o LOGLEVEL=DEBUG3 \
  -o ssh_command="ssh -vv" -d %USER%@%HOST%:/../../.. X:
```

## Issues

### Windows 7 x64, WinFsp 2018.2 B1 , SSHFS-Win v2.7.17334

1.  The only way to remove the warning "cannot create /home/user/.ssh foder" is to pass `-F sshfs_config` file
2.  `-F %USERPROFILE%\\sshfs_config` works, but it does not if the parameter is created by sshfs-win.exe
3.  The sshfs_config file must have forward slashes, and variables are not expanded. 
    `%USERPROFILE%`, or `C:\\Users\\\user\\sshfs_config` does not work, but `C:/Users/user/sshfs_config` does work.
4.  The bash home expansion variable `'~'` works in cygwin terminal, but not in cmd. 
    When running sshfs.exe from a cmd terminal, the `~/.ssh/id_rsa` path is not found by ssh.
5.  The `-o mask=007` does not work, the `-o create_mask` does work, but not in all Linux servers. 
    It seems to be overriten by some server configuration?
6.  Consider implementing a drive name, which is different from volname and fylesystemname. 
    It seems to be set with this registry value:
    ```
    set DRIVENAME=Linux
    set REGKEY=HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer\MountPoints2\##sshfs#%USER%@%HOST%
    reg add %REGKEY% /v _LabelFromReg /d %DRIVENAME% /f
    ```
7.  Consider unifying log files and parameters
8.  Another user can see the mount, but not access the files, the Server permissions seem to apply. 
    However, the other user can disconnect the drive.
9.  The ssh from cygwin has a hardcoded `/home/user/.ssh` location, setting HOME to %USERPROFILE% does not work. 
    The ssh from msysgit works fine.
10. Consider adding ssh-copy-id program
11. Disconected drive does not reconnect. After investigation, the programs run by SYSTEM were not running. 
    Rebooting solved the issue.
12. Mounting the root directory as specified by the readme does not work: 
    `\\sshfs\user@linux` mounts my home
    `\\sshfs\user@linux\` mounts my home
    `\\sshfs\user@linux\\home\user`
    `\\sshfs\user@linux\\`
    `\\sshfs\user@linux\`
    This format does work: 
    `\\sshfs\user@linux\..\..`
    Any path further up the root will work, such as `\\sshfs\user@linux\..\..\..\..\..\..`
    From command line is just needed this: `sshfs.exe user@linux:/`

### Windows 7 x64, WinFsp v1.4B2, SSHFS-Win v3.2.18213

13. -o VolumePrefix in use, error with Status=80070050, reported as issue 44.
14. `> net use z: /delete` does not kill processes run by SYSTEM, drive remains in disconnected status. 
    The only way to cleanup is clean registry and mount from command line using sshfs, then kill the process.
    It is hard to disconnect the drive from windows explorer, it does not disconnect.
    I have to kill the sshfs process run by SYSTEM, then the drive stays in error status. The only way to remove it is to mount with the command line and kill the process in the terminal with Control+C.
    When mounting using `> net use Z: \\sshfs\user@host` it requires two commands to unmount:
    `> net use Z: /del`
    and
    `> net use \\sshfs\user@host /del`
    in any order.
    I also noticed that the remote path must be the original path that was used to mount, stored in the registry `HKCU\Network`, not the path reported by the > net use command.
15. BUG: copy files between 2 drives supported?
16. New version with fuse3 unstable, copy/paste folder crashes sshfs process, copy file hangs explorer.
17. Resource temporarily unavailable, bug


## Error codes:

- 0x800704b3 the server is down, path incorrect
- Wrong path: no such file or directory
- Cannot set WinFsp-FUSE file system mount point. 
  
  The service sshfs has failed to start (Status=c00000ca): Drive letter incorrect
  
  The service sshfs has failed to start (Status=80070050): VolumePrefix is in use
  
  The service sshfs has failed to start (Status=c000000d): VolumePrefix with backslashes


## BUG01

I am using the command line with these arguments:

```
C:\Program Files\SSHFS-Win\bin>sshfs.exe %USER%@%HOST%:/../..  X: -o VolumePrefix=/sshfs/%USER%@%HOST% -o rellinks -o uid=-1,gid=-1,create_umask=0007 -o FileSystemName=SSHFS -o reconnect -f -F c:/users/user/.ssh/config
```

The config file has this:

```
Host *
   ServerAliveInterval 60
   UserKnownHostsFile=C:/Users/user/.ssh/known_hosts
   StrictHostKeyChecking no
   IdentityFile C:/Users/user/.ssh/id_rsa
```

After some inactivity, the server disconnects and I get the error below, with the X: drive unavailable.  

```
Connection reset by 192.168.56.100 port 22
remote host has disconnected
read: Software caused connection abort
read: Software caused connection abort
      2 [main] sshfs 12904 C:\Program Files\SSHFS-Win\bin\sshfs.exe: *** fatal error in forked process - failed to create new win32 semaphore, currentvalue 4294967295, Win32 error 87
Stack trace:
Frame        Function    Args
0000546C168  0018005C9FE (00180253992, 0018021BC46, 00000000057, 0000546B010)
0000546C861  7FFD89383B88 (7FFD893A07D8, 000027B5B20, 00000000000, 00000000000)
End of stack trace (more stack frames may be present)
read: Software caused connection abort
```


# Build ssh with visual studio

1. Install Active Perl (perl in cygwin didn't work). 
2. Build OpenSSL. Download latest source code tar.gz and untar. In a VS terminal go to the source folder and run:

   ```
   > perl Configure VC-WIN64A no-asm --prefix=C:\OpenSSL
   > ms\do_win64a
   > nmake -f ms\nt.mak install
   ```
3. Rename libraries located in c:\OpenSSL\lib:
   ```
   libeay32.lib -> libcypto.lib
   ssleay32.lib -> libssl.lib 
   ```
2. Clone openssh for windows at https://github.com/PowerShell/openssh-portable.git
3. Open "contrib\win32\openssh\Win32-OpenSSH.sln" in visual studio. Retarget if needed. Add additional includes: c:\OpenSSL\include, and Additionals link libraries: c:\OpenSSL\lib.
4. Build libssh, openbsd_compat, posix_compat, ssh, and ssh-keygen.


# Feature request: mount root by default
This is related to issues...
sshfs-win.exe creates a sshfs.exe call with arguments. Let's blame windows first. The problem starts when we use `user@host\\path`. Using net use from from command line translates to `user@host:/` and mounts the root path, but using the network mapping drive form windows explorer, double slashes are transformed into one which translates into user@host: and mounts the home directory. 
The solution is to mount the root path by default. This will make the mounting path consistent and will remove the current confusion. Anyone using this technology is already familiar with the Linux file system and this change will not produce any friction. Even if a user needs to mount the home directory and has zero knowledge of Linux, the home full path can be provided to him/her by the IT guys.

| sshfs-win.exe       | translates to | should be |
| -------------       |---------------| ----------|
| `user@host`         | `user@host:`    | user@host:/ |
| `user@host\`        | `user@host:`    | user@host:/ |
| `user@host\\`       | `user@host:/` if net use, but <br> `user@host:` if map drive | user@host:/ |
| `user@host\\path`   | `user@host:/path` if net use, but <br> `user@host:path` if map drive | user@host:/path |
| `user@host\\\\\`    | `user@host:///` | user@host:/ |
| `user@host\..\`     | `user@host:../` | user@host:/../ |
