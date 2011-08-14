CC = c99
CFLAGS = -Wall -g

.PHONY: all clean

all: squareciv

squareciv: squareciv.c point.o dwarf.o task.o map.o menu.o building.o orders.o
	$(CC) $(CFLAGS)      -o $@ $^ -ltcod

clean:
	rm -f squareciv *.o
