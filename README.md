# Estação Noturna: Sinal de Ruptura

Jogo de investigação e suspense feito em C com raylib para a disciplina de AED. Você opera uma rádio
clandestina durante quatro noites e decide o que transmitir, em que ordem e em quem confiar.

## Como executar (Windows)

1. Instale o MSYS2 (https://www.msys2.org) e abra o terminal **MSYS2 UCRT64**.
2. Instale o compilador e a raylib:

```bash
pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-raylib mingw-w64-ucrt-x86_64-pkgconf make
```

3. Entre na pasta do projeto e compile:

```bash
cd /c/caminho/da/pasta/EstacaoNoturna
make
```

4. Rode o jogo:

```bash
./build/estacao_noturna.exe
```

Para rodar os testes das estruturas de dados e das regras: `make test`.

Fora do terminal do MSYS2, o executável precisa de `C:\msys64\ucrt64\bin` no `PATH` (DLL da raylib).

## Controles

- `WASD` ou setas: andar
- `E`: usar o objeto próximo
- `TAB`: tarefas da noite
- `ESC`: fechar painel / pausa
- `F11`: tela cheia
- No transmissor: `Cima`/`Baixo` escolhem, `Shift+Cima`/`Shift+Baixo` mudam a ordem, `P` prioriza,
  `R` tira do roteiro e `ENTER` transmite
- Tudo nos painéis também funciona com o mouse.

## Estruturas de dados e ordenação

- **Lista duplamente encadeada** (`programacao.c`): o roteiro da rádio. Cada nó é uma transmissão, e a
  ordem dos nós é a ordem em que elas vão ao ar. Inserir, remover e mover nós muda o que acontece no jogo.
- **Fila encadeada (FIFO)** (`programacao.c`): os sinais que chegam ao receptor, na ordem de chegada.
- **Insertion Sort** (`ordenarPorPrazo` em `programacao.c`): o botão "Priorizar urgentes" ordena o roteiro
  pelo prazo. Depois o jogador ainda precisa ajustar a ordem para as senhas irem ao ar antes dos avisos.

## Arquivos

| Arquivo | Conteúdo |
|---|---|
| `programacao.c/h` | Lista duplamente encadeada, Insertion Sort e fila |
| `partida.c/h` | Regras do jogo |
| `campanha.c` | Textos e dados das quatro noites |
| `main.c` | Menu, telas e a função `main` |
| `noite.c` | HUD e painéis dos objetos |
| `cabine.c` | Cabine, colisão e personagem |
| `graficos.c` | Texto, pixel art, sprites e botões |
| `audio.c` | Música e efeitos |
| `interface.h` | Declarações da parte gráfica |
| `tests/testes.c` | Testes |
