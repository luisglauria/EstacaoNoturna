CC ?= gcc
TARGET := build/estacao_noturna.exe
SRC := main.c game.c player.c cabin.c ui.c radio_list.c
OBJ := $(SRC:%.c=build/%.o)

CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -O2
RAYLIB_CFLAGS ?= $(shell pkg-config --cflags raylib 2>/dev/null)
RAYLIB_LIBS ?= $(shell pkg-config --libs raylib 2>/dev/null)

ifeq ($(strip $(RAYLIB_LIBS)),)
RAYLIB_LIBS := -lraylib -lopengl32 -lgdi32 -lwinmm
endif

all: $(TARGET)

$(TARGET): $(OBJ)
	@mkdir -p build
	$(CC) $(CFLAGS) $(OBJ) -o $@ $(RAYLIB_LIBS)

build/%.o: %.c
	@mkdir -p build
	$(CC) $(CFLAGS) $(RAYLIB_CFLAGS) -c $< -o $@

clean:
	rm -rf build

.PHONY: all clean
