CC = gcc

INC="-IC:\Users\kamila\Desktop\tools\PDCurses-3.9"
LDD="-LC:\Users\kamila\Desktop\tools\PDCurses-3.9\wincon"
LDFLAGS = -lpdcurses

CFLAGS = -Wall -mconsole $(INC)
SRCS = main.c interface.c history.c
OBJS = $(SRCS:.c=.o)

all: history_test.exe

history_test.exe: $(OBJS)
	$(CC) $(OBJS) -o history_test.exe $(LDD) $(LDFLAGS) 

history.o: history.c history.h
	$(CC) $(CFLAGS) -c history.c -o history.o

interface.o: interface.c interface.h 
	$(CC) $(CFLAGS) -c interface.c -o interface.o

clean:
	if exist *.o del *.o
	if exist history_test.exe del history_test.exe
run: history_test.exe
	history_test.exe

rebuild: clean all

.PHONY: all clean run rebuild