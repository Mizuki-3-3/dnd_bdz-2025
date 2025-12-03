CC = gcc

INC="-IC:\Users\kamila\Desktop\tools\PDCurses-3.9"
LDD="-LC:\Users\kamila\Desktop\tools\PDCurses-3.9\wincon"
LDFLAGS = -lpdcurses

CFLAGS = -Wall -Wextra -std=c99 $(INC)

SRCS = main.c hero.c inventory.c player_actions.c interface.c monster.c
OBJS = $(SRCS:.c=.o)

all: prog.exe

prog.exe: $(OBJS)
	$(CC) $(OBJS) -o prog.exe $(LDD) $(LDFLAGS)

main.o: main.c hero.h player_actions.h inventory.h
	$(CC) $(CFLAGS) -c main.c -o main.o

hero.o: hero.c hero.h equipment.h
	$(CC) $(CFLAGS) -c hero.c -o hero.o

inventory.o: inventory.c inventory.h equipment.h hero.h
	$(CC) $(CFLAGS) -c inventory.c -o inventory.o

player_actions.o: player_actions.c player_actions.h hero.h inventory.h equipment.h
	$(CC) $(CFLAGS) -c player_actions.c -o player_actions.o

interface.o: interface.c interface.h inventory.h equipment.h
	$(CC) $(CFLAGS) -c interface.c -o interface.o

monster.o: monster.c monster.h
	$(CC) $(CFLAGS) -c monster.c -o monster.o

clean:
	if exist *.o del *.o
	if exist prog.exe del prog.exe

run: prog.exe
	prog.exe

rebuild: clean all

.PHONY: all clean run rebuild