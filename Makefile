CC=gcc
INC="-IC:\Users\kamila\Desktop\tools\PDCurses-3.9"
LDD="-LC:\Users\kamila\Desktop\tools\PDCurses-3.9\wincon"
LDFLAGS="-lpdcurses"

prog.exe: hello.o
	$(CC) $^ $(LDD) $(LDFLAGS) -o prog.exe 

hello.o: hello.c
	$(CC) $(INC) -c $^ -o $@
clean:
			rm -rf hello *.o