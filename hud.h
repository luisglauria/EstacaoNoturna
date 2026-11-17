#ifndef HUD_H
#define HUD_H

/* Informações fixas sobre a cabine: barra do topo, rodapé, alertas, tarefas (TAB), balões e avisos. */

#include "partida.h"
#include "raylib.h"
#include <stdbool.h>

#define HUD_ALTURA_TOPO 60
#define HUD_Y_RODAPE 684

void hud_desenhar_topo(const Partida *partida);
void hud_desenhar_rodape(const Partida *partida, bool painel_aberto);
/* Faixa vermelha com a contagem do holofote ou da inspeção. */
void hud_desenhar_alerta(const Partida *partida, float tempo);
/* Dica de como cumprir a próxima obrigatória (configuração "Dicas na tela"). */
void hud_desenhar_dica(const Partida *partida);
/* Lista completa de tarefas obrigatórias e opcionais (aberta com TAB). */
void hud_desenhar_tarefas(const Partida *partida);
void hud_desenhar_balao(Vector2 posicao, const char *rotulo, float tempo);
/* Aviso com o resultado da última ação; alpha de 0 a 1. Com painel aberto vai na faixa de baixo dele. */
void hud_desenhar_aviso(const char *texto, float alpha, bool em_painel);

#endif
