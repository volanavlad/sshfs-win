import subprocess
import time

ssh_cyg = r'"C:\Program Files\SSHFS-Win\bin\ssh.exe"'
ssh_vs = r'C:\Users\sant\Documents\openssh-portable\bin\x64\Release\ssh.exe'

cmd = 'cmd /c {} 10.0.0.10 hostname'
cmd = cmd.format(ssh_cyg)
t = time.time()
for i in range(10):
	stdout = subprocess.check_output(cmd, shell=True).strip()
	assert stdout.decode('utf-8') == 'box'
print('ssh cygwin time elapsed: {} secs.'.format(time.time()-t))

cmd = cmd.format(ssh_vs)
t = time.time()
for i in range(10):
	stdout = subprocess.check_output(cmd, shell=True).strip()
	assert stdout.decode('utf-8') == 'box'
print('ssh vs time elapsed: {} secs.'.format(time.time()-t))