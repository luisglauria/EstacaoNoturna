#ifndef PIXEL_H
#define PIXEL_H

/* Mundo em pixel art: a cabine é desenhada numa tela pequena (427x240) e ampliada 3x com pixels
   nítidos. A luz é um "mapa de iluminação" multiplicado sobre a cena: fora das luzes fica escuro. */

#include "raylib.h"
#include <stdbool.h>

#define PIXEL_ESCALA 3
#define MUNDO_LARGURA 427
#define MUNDO_ALTURA 240

/* Converte coordenada da tela virtual (1280x720) para pixel do mundo. */
int px(float coordenada_virtual);
/* Converte pixel do mundo para coordenada da tela virtual. */
float px_virtual(int pixel);

void pixel_iniciar(void);
void pixel_encerrar(void);

/* ---------- passe de luz (antes da cena) ---------- */
void pixel_luz_comecar(Color ambiente);
/* Luz redonda com degradê pontilhado; forca de 0 a 1. */
void pixel_luz(float x, float y, float raio, Color cor, float forca);
/* Feixe em cone saindo de origem na direção de alvo (janela -> chão). */
void pixel_feixe(Vector2 origem, Vector2 alvo, float largura, Color cor, float forca);
void pixel_luz_terminar(void);

/* ---------- passe da cena ---------- */
void pixel_mundo_comecar(Color fundo);
/* Multiplica o mapa de luz sobre o que já foi desenhado; depois disso, desenhe só o que brilha. */
void pixel_aplicar_luz(void);
void pixel_vinheta(float forca);
/* Fecha o mundo e o coloca na tela virtual, ampliado 3x. */
void pixel_mundo_terminar(void);

/* ---------- primitivas em pixels do mundo ---------- */
void px_ret(int x, int y, int largura, int altura, Color cor);
void px_borda(int x, int y, int largura, int altura, Color cor);
void px_hlinha(int x, int y, int largura, Color cor);
void px_vlinha(int x, int y, int altura, Color cor);
void px_ponto(int x, int y, Color cor);
/* Móvel visto de cima em 3/4: tampo, frente (altura_frente px), contorno escuro e brilho na borda. */
void px_movel(int x, int y, int largura, int altura, int altura_frente, Color tampo, Color frente);
/* Ruído determinístico por posição (texturas que não "tremem" entre quadros). */
unsigned px_ruido(int x, int y);
Color px_escurecer(Color cor, float fator);

#endif
