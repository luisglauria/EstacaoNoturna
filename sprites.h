#ifndef SPRITES_H
#define SPRITES_H

/* Sprites em pixel art definidos como mapas de letras (cada letra é uma cor da paleta).
   Viram texturas na inicialização; nada é carregado de arquivo. */

typedef enum { DIRECAO_BAIXO, DIRECAO_CIMA, DIRECAO_ESQUERDA, DIRECAO_DIREITA } Direcao;
typedef enum { SPRITE_FOTO, SPRITE_RADIO_DOMESTICO, SPRITE_TOTAL } SpriteObjeto;

#define OPERADOR_LARGURA 16
#define OPERADOR_ALTURA 24

void sprites_iniciar(void);
void sprites_encerrar(void);
/* Desenha o operador com o centro dos pés em (x, y_pes), em pixels do mundo.
   passo: 0 parado; 1 a 3 alternam as pernas ao andar. */
void sprite_operador(int x, int y_pes, Direcao direcao, int passo);
void sprite_objeto(SpriteObjeto objeto, int x, int y);

#endif
