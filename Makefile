CC = gcc

INC="-IC:\Users\kamila\Desktop\tools\PDCurses-3.9"
LDD="-LC:\Users\kamila\Desktop\tools\PDCurses-3.9\wincon"
LDFLAGS = -lpdcurses

CFLAGS = -Wall -Wextra -std=c99 $(INC)

SRCS = main.c interface.c
OBJS = $(SRCS:.c=.o)

all: prog.exe 

prog.exe: $(OBJS)
	$(CC) $(OBJS) -o prog.exe $(LDD) $(LDFLAGS)

main.o: main.c interface.h
	$(CC) $(CFLAGS) -c main.c -o main.o



interface.o: interface.c interface.h
	$(CC) $(CFLAGS) -c interface.c -o interface.o


clean:
	if exist *.o del *.o
	if exist prog.exe del prog.exe

run: prog.exe
	prog.exe

rebuild: clean all

.PHONY: all clean run rebuild