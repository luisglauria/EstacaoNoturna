# O make define CC=cc por padrão, então "CC ?= gcc" nunca valia; troca só o padrão.
ifeq ($(origin CC),default)
CC := gcc
endif

TARGET := build/estacao_noturna.exe

# Regras e estruturas de dados: não usam raylib e são as que os testes compilam.
LOGICA := programacao.c fila_mensagens.c campanha.c partida.c partida_acoes.c configuracoes.c
# Interface gráfica (raylib).
INTERFACE := main.c app.c telas.c fundo.c tutorial.c resumo.c game.c player.c cabin.c cabin_props.c \
             hud.c widgets.c painel_receptor.c painel_bancada.c painel_transmissor.c painel_ambiente.c \
             render.c pixel.c sprites.c audio.c
SRC := $(INTERFACE) $(LOGICA)
OBJ := $(SRC:%.c=build/%.o)

CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -O2
RAYLIB_CFLAGS ?= $(shell pkg-config --cflags raylib 2>/dev/null)
RAYLIB_LIBS ?= $(shell pkg-config --libs raylib 2>/dev/null)

ifeq ($(strip $(RAYLIB_LIBS)),)
RAYLIB_LIBS := -lraylib -lopengl32 -lgdi32 -lwinmm
endif

TESTES := build/test_programacao.exe build/test_fila.exe build/test_partida.exe

all: $(TARGET)

$(TARGET): $(OBJ)
	@mkdir -p build
	$(CC) $(CFLAGS) $(OBJ) -o $@ $(RAYLIB_LIBS)

build/%.o: %.c
	@mkdir -p build
	$(CC) $(CFLAGS) $(RAYLIB_CFLAGS) -c $< -o $@

# Os testes não dependem da raylib.
build/test_programacao.exe: tests/test_programacao.c programacao.c programacao.h tests/verificacao.h
	@mkdir -p build
	$(CC) $(CFLAGS) tests/test_programacao.c programacao.c -o $@

build/test_fila.exe: tests/test_fila.c fila_mensagens.c fila_mensagens.h tests/verificacao.h
	@mkdir -p build
	$(CC) $(CFLAGS) tests/test_fila.c fila_mensagens.c -o $@

build/test_partida.exe: tests/test_partida.c $(LOGICA) partida.h partida_interno.h campanha.h tests/verificacao.h
	@mkdir -p build
	$(CC) $(CFLAGS) tests/test_partida.c $(LOGICA) -o $@

test: $(TESTES)
	./build/test_programacao.exe
	./build/test_fila.exe
	./build/test_partida.exe

clean:
	rm -rf build

.PHONY: all test clean
