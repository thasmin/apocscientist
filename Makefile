CC = gcc -std=c99
OS = $(shell uname -s | tr a-z A-Z)
CFLAGS = -g -Wall -Werror
LDFLAGS = -ltcod
ifeq ($(OS),DARWIN)
CFLAGS += -DMACOSX
LDFLAGS += -lpng -L/usr/X11/lib -framework OpenGL -lSDL
endif

.PHONY: all clean

all: squareciv

squareciv: squareciv.c point.o robot.o task.o map.o menu.o building.o orders.o job.o
	$(CC) $(CFLAGS)      -o $@ $^ $(LDFLAGS)

clean:
	rm -f squareciv *.o
