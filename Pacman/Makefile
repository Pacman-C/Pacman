CC      = gcc
CFLAGS  = -Wall -Wextra -Iinclude $(shell sdl2-config --cflags)
LIBS    = $(shell sdl2-config --libs) -lm
SRC_DIR = src
OBJ_DIR = obj
SRC     = $(wildcard $(SRC_DIR)/*.c)
OBJ     = $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

ifeq ($(OS),Windows_NT)
    TARGET   = pacman.exe
    COPY_DLL = $(if $(wildcard lib/SDL2.dll), cp lib/SDL2.dll ., )
else
    TARGET   = pacman
    COPY_DLL =
endif

all: $(OBJ_DIR) $(TARGET)
	$(COPY_DLL)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(TARGET)

.PHONY: all clean