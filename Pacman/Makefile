CC      = gcc
CFLAGS  = -Wall -Wextra -Iinclude $(shell sdl2-config --cflags)
LIBS    = $(shell sdl2-config --libs) -lm
SRC_DIR = src
OBJ_DIR = obj
SRC     = $(wildcard $(SRC_DIR)/*.c)
OBJ     = $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
TARGET  = pacman

all: $(TARGET)
	-cp C:/msys64/mingw64/bin/SDL2.dll .

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(TARGET)

.PHONY: all clean