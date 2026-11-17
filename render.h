#ifndef RENDER_H
#define RENDER_H

#include "raylib.h"

/* O jogo desenha sempre nesta resolução e a imagem é escalada para a janela. */
#define VIRTUAL_WIDTH 1280
#define VIRTUAL_HEIGHT 720

/* Paleta única do jogo: toda cor de interface sai daqui. */
/* Tons frios e escuros, com âmbar (luz de abajur), verde (tela de rádio) e vermelho (perigo). */
#define COLOR_NIGHT ((Color){6, 7, 11, 255})
#define COLOR_BAR ((Color){10, 11, 16, 255})
#define COLOR_PANEL ((Color){15, 16, 23, 255})
#define COLOR_CARD ((Color){26, 28, 38, 255})
#define COLOR_CARD_SELECTED ((Color){62, 48, 30, 255})
#define COLOR_EDGE ((Color){132, 100, 64, 255})
#define COLOR_CREAM ((Color){226, 214, 186, 255})
#define COLOR_MUTED ((Color){118, 122, 138, 255})
#define COLOR_AMBER ((Color){232, 176, 84, 255})
#define COLOR_SIGNAL ((Color){124, 214, 164, 255})
#define COLOR_ALERT ((Color){214, 70, 62, 255})

void render_init(void);
void render_shutdown(void);
/* Tudo entre begin e end é desenhado na tela virtual. */
void render_begin_frame(void);
void render_end_frame(void);
/* Saem e voltam da tela virtual, para desenhar em outra textura (o mundo em pixel art) no meio do quadro. */
void render_pause_frame(void);
void render_resume_frame(void);
/* Copia uma textura de render para a área, sem misturar alfa (evita o vazamento do fundo). */
void render_copy_texture(Texture2D texture, Rectangle dest);
/* Posição do mouse já convertida para coordenadas da tela virtual. */
Vector2 render_mouse_position(void);
void render_text(const char *text, float x, float y, float size, Color color);
void render_text_centered(const char *text, float center_x, float y, float size, Color color);
float render_text_width(const char *text, float size);
/* Desenha com quebra de linha por palavras dentro da largura da área; devolve a altura usada. */
float render_text_wrapped(const char *text, Rectangle area, float size, Color color);
/* Só mede a altura que o texto ocuparia, sem desenhar. */
float render_text_wrapped_height(const char *text, float width, float size);

#endif
