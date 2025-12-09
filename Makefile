CC = gcc

INC="-IC:\Users\kamila\Desktop\tools\PDCurses-3.9"
LDD="-LC:\Users\kamila\Desktop\tools\PDCurses-3.9\wincon"
LDFLAGS = -lpdcurses

CFLAGS = -Wall -mconsole $(INC)
SRCS = main.c inventory.c equipment.c
OBJS = $(SRCS:.c=.o)

all: inventory_test.exe

inventory_test.exe: $(OBJS)
	$(CC) $(OBJS) -o inventory_test.exe $(LDD) $(LDFLAGS) 


equipment.o: equipment.c equipment.h
	$(CC) $(CFLAGS) -c equipment.c -o equipment.o

inventory.o: inventory.c inventory.h equipment.h
	$(CC) $(CFLAGS) -c inventory.c -o inventory.o

clean:
	if exist *.o del *.o
	if exist inventory_test.exe del inventory_test.exe
run: inventory_test.exe
	inventory_test.exe

rebuild: clean all

.PHONY: all clean run rebuild