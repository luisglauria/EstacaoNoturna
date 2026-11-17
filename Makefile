# O make define CC=cc por padrão, então "CC ?= gcc" nunca valeria; troca só o padrão.
ifeq ($(origin CC),default)
CC := gcc
endif

TARGET := build/estacao_noturna.exe

# Regras e estruturas de dados: não usam raylib e são o que os testes compilam.
LOGICA := programacao.c campanha.c partida.c
# Interface gráfica (raylib).
INTERFACE := main.c noite.c cabine.c graficos.c audio.c
OBJ := $(patsubst %.c,build/%.o,$(INTERFACE) $(LOGICA))

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

build/%.o: %.c programacao.h partida.h interface.h
	@mkdir -p build
	$(CC) $(CFLAGS) $(RAYLIB_CFLAGS) -c $< -o $@

build/testes.exe: tests/testes.c $(LOGICA) programacao.h partida.h
	@mkdir -p build
	$(CC) $(CFLAGS) tests/testes.c $(LOGICA) -o $@

test: build/testes.exe
	./build/testes.exe

clean:
	rm -rf build

.PHONY: all test clean
