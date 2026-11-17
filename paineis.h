#ifndef PAINEIS_H
#define PAINEIS_H

/* Painéis contextuais abertos com E perto de cada objeto. Cada função desenha o painel dentro
   da área de conteúdo e trata os próprios cliques e atalhos (interface em modo imediato). */

#include "game.h"
#include "raylib.h"

void painel_receptor(GameState *game, Rectangle area);
void painel_bancada(GameState *game, Rectangle area);
void painel_transmissor(GameState *game, Rectangle area);
void painel_frequencia(GameState *game, Rectangle area);
void painel_janela(GameState *game, Rectangle area);
void painel_porta(GameState *game, Rectangle area);
void painel_colchao(GameState *game, Rectangle area);
void painel_objeto(GameState *game, Rectangle area);

#endif
