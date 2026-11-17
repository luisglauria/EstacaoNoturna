# Estação Noturna: Sinal de Ruptura

Jogo de investigação e suspense em C com raylib, feito para a disciplina de AED. Você opera uma
rádio clandestina durante quatro noites, decide em quem confiar e em que ordem transmitir.

## Como compilar e jogar

O projeto usa raylib 5.x. No MSYS2 UCRT64:

```bash
pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-raylib mingw-w64-ucrt-x86_64-pkgconf make
```

No terminal **MSYS2 UCRT64**, dentro da pasta do projeto:

```bash
make
./build/estacao_noturna.exe
```

Testes das estruturas de dados e das regras (não precisam da raylib):

```bash
make test
```

Fora do terminal MSYS2, o executável precisa de `C:\msys64\ucrt64\bin` no `PATH` (DLLs da raylib).
As configurações ficam em `settings.cfg`, criado na pasta de onde o jogo é executado.

## Controles

- `WASD` ou setas: andar · `E`: usar o objeto próximo · `TAB`: tarefas · `ESC`: fechar painel / pausa · `F11`: tela cheia
- Transmissor: `Cima`/`Baixo` selecionam, `Shift+Cima`/`Shift+Baixo` movem, `P` prioriza, `R` remove, `ENTER` transmite a próxima
- Tudo nos painéis também funciona com o mouse.

## Como o jogo funciona

Cada noite tem **preparação** (o tempo não corre: ler novidades, consultar documentos, escolher um
preparativo), **operação** (a rádio no ar; cada ação com custo gasta pontos de ação, mostrados no jogo
como bateria e relógio: 1 PA = 15 minutos, a noite começa às 23:00) e **encerramento** (resumo com
transmissões, tarefas, consequências, contatos, provas e recursos). Tarefas obrigatórias e opcionais
ficam no `TAB`; obrigatórias não feitas geram consequências.

| Ação | Custo |
|---|---|
| Andar, ler, comparar documento × mensagem, aceitar/riscar, reorganizar, priorizar | sem custo |
| Confirmar mensagem, investigar, trocar frequência, cortina, esconder papéis, desligar equipamentos | 1 PA (15 min) |
| Transmitir | custo da transmissão (1 a 3 PA) |

Dentro do jogo não aparecem termos de programação: a lista é o "roteiro da noite" e a fila são os
"sinais em espera". Os nomes técnicos ficam no código e neste README.

## Arte e som

- **Pixel art**: a cabine é desenhada numa tela de 427×240 e ampliada 3× com pixels nítidos, vista de
  cima em 3/4. A luz é um mapa de iluminação com degradês pontilhados multiplicado sobre a cena
  (abajur, telas dos equipamentos, luar, relâmpagos, holofote). Os sprites (operador em 4 direções com
  passos, fotografia, rádio doméstico) são mapas de letras em `graficos.c`, convertidos em textura na
  inicialização.
- **Música e efeitos** são sintetizados em `audio.c`: uma camada de suspense (zumbido grave, acorde
  menor, chiado de rádio, Morse distante) e uma camada de tensão (coração, cordas em trêmulo) que
  cresce quando o holofote ou a revista se aproximam. Efeitos: passos, trovão, sirene, batidas na porta.

Emergências: **holofote** (noites 2 e 3: feche a cortina ou saia da área marcada) e **inspeção**
(noite 4: esconda as provas, desligue os equipamentos e responda à porta). Finais: denúncia completa,
denúncia incompleta ou captura. O plano completo e as decisões de projeto estão em `docs/PLANO.md`.

## Estruturas de dados (AED)

- **Lista duplamente encadeada** (`programacao.c`): a programação da rádio. Cada nó guarda id, código,
  texto, custo, prazo, dependência (`requer`) e consequência. Funções: `inicializarProgramacao`,
  `inserirTransmissao`, `buscarTransmissao`, `removerTransmissao`, `moverTransmissao`,
  `exibirProgramacao` (a interface desenha a lista por ela), `ordenarPorPrazo` (Insertion Sort
  estável que religa os nós), `executarProximaTransmissao` e `liberarProgramacao`.
- **Fila FIFO encadeada** (também em `programacao.c`): mensagens chegando ao receptor, na ordem de chegada.

A ordem tem efeito real: uma transmissão falha se o prazo passou, se o código de que ela depende não
foi ao ar antes, ou se a mensagem era falsa. "Priorizar" ordena só pelo prazo, então depois é preciso
ajustar a ordem à mão para respeitar as dependências.

## Arquivos

| Responsabilidade | Arquivos |
|---|---|
| Estruturas de dados (lista e fila) | `programacao.c/h` |
| Regras, eventos e emergências (sem raylib) | `partida.c/h` |
| Dados narrativos (textos das 4 noites) | `campanha.c` |
| Declarações da parte gráfica | `interface.h` |
| Menu, telas, configurações e `main` | `main.c` |
| Noite na tela: HUD, painéis dos objetos | `noite.c` |
| Cabine, móveis, colisão e personagem | `cabine.c` |
| Tela virtual, texto, pixel art, sprites e componentes | `graficos.c` |
| Música e efeitos sonoros | `audio.c` |
| Testes | `tests/testes.c` |
