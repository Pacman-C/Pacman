CC      = gcc
CFLAGS  = -Wall -Iinclude $(shell sdl2-config --cflags)
LIBS    = $(shell sdl2-config --libs)
SRC     = $(wildcard src/*.c)

pacman: $(SRC)
    $(CC) $(CFLAGS) $^ -o $@ $(LIBS)

clean:
    rm -f pacman