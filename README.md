# Estação Noturna: Sinal de Ruptura

Protótipo em C com raylib para a disciplina de AED.

## Dependência

O projeto usa raylib 5.x. No MSYS2 UCRT64, instale a biblioteca com:

```bash
pacman -S mingw-w64-ucrt-x86_64-raylib
```

Compile no terminal **MSYS2 UCRT64**:

```bash
make
./build/estacao_noturna.exe
```

Se a raylib estiver em outro local, ajuste `RAYLIB_CFLAGS` e `RAYLIB_LIBS` no `Makefile`.

## Controles

- `WASD` ou setas: movimentar
- `E`: interagir com o equipamento próximo
- `ESC`: fechar painel
- No transmissor: `P` prioriza transmissões urgentes; `R` remove a transmissão selecionada
- No transmissor: `TAB` seleciona a próxima transmissão

## Estrutura

- `main.c`: ponto de entrada
- `game.c`: dono do `GameState` e ciclo principal
- `player.c`: movimento e colisão
- `cabin.c`: geometria da cabine e pontos de interação
- `ui.c`: renderização retro e painéis
- `radio_list.c`: lista duplamente encadeada e Insertion Sort

