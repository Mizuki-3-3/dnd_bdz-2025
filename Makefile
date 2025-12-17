CC = gcc

INC="-IC:\Users\kamila\Desktop\tools\PDCurses-3.9"
LDD="-LC:\Users\kamila\Desktop\tools\PDCurses-3.9\wincon"
LDFLAGS = -lpdcurses

CFLAGS = -Wall -Wextra -O2 -mconsole $(INC) -D_UNICODE -DUNICODE -DPDC_FORCE_UTF8
SRCS = main.c game.c interface.c history.c inventory.c hero.c database.c monster.c combat.c
OBJS = $(SRCS:.c=.o)

all: test.exe

test.exe: $(OBJS)
	$(CC) $(OBJS) -o test.exe $(LDD) $(LDFLAGS)

main.o: main.c game.h
	$(CC) $(CFLAGS) -c main.c -o main.o

game.o: game.c game.h interface.h history.h hero.h inventory.h database.h monster.h location.h
	$(CC) $(CFLAGS) -c game.c -o game.o

interface.o: interface.c interface.h inventory.h equipment.h hero.h database.h
	$(CC) $(CFLAGS) -c interface.c -o interface.o

history.o: history.c history.h
	$(CC) $(CFLAGS) -c history.c -o history.o

inventory.o: inventory.c inventory.h equipment.h database.h hero.h
	$(CC) $(CFLAGS) -c inventory.c -o inventory.o

hero.o: hero.c hero.h inventory.h
	$(CC) $(CFLAGS) -c hero.c -o hero.o

database.o: database.c database.h monster.h equipment.h location.h
	$(CC) $(CFLAGS) -c database.c -o database.o

monster.o: monster.c monster.h
	$(CC) $(CFLAGS) -c monster.c -o monster.o

combat.o: combat.c combat.h hero.h monster.h
	$(CC) $(CFLAGS) -c combat.c -o combat.o
clean:
	if exist *.o del *.o
	if exist test.exe del test.exe
	if exist *.save del *.save
run: test.exe
	test.exe

rebuild: clean all

.PHONY: all clean run rebuild