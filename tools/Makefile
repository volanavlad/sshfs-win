# my sshfs-win makefile

CC=gcc
CFLAGS=-I.
DEPS = jsmn.h
OBJ = sshfs-win.o jsmn.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

sshfs-win: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm -f *.o 
	rm -f *.exe
	
.PHONY: clean

