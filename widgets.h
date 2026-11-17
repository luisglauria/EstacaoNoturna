#ifndef WIDGETS_H
#define WIDGETS_H

/* Componentes de interface no estilo "modo imediato": cada função desenha o componente e
   devolve true no quadro em que ele foi clicado/alterado. Coordenadas na tela virtual. */

#include "raylib.h"
#include <stdbool.h>

/* Função chamada a cada clique em botão (o app liga aqui o som de clique). */
void ui_definir_som_clique(void (*tocar)(void));

bool ui_mouse_sobre(Rectangle area);
bool ui_clicou(Rectangle area);
bool ui_botao(Rectangle area, const char *rotulo, bool habilitado);
/* Botão com etiqueta de custo à direita ("1 PA" ou "grátis"), para o custo aparecer antes de confirmar. */
bool ui_botao_custo(Rectangle area, const char *rotulo, int custo, bool habilitado);
/* Botão de destaque (ação principal do painel). */
bool ui_botao_principal(Rectangle area, const char *rotulo, int custo, bool habilitado);
/* Item de lista selecionável com título e linha secundária. */
bool ui_item_lista(Rectangle area, const char *titulo, const char *detalhe, bool selecionado, Color cor_detalhe);
void ui_etiqueta(float x, float y, const char *texto, Color cor);
void ui_medidor(Rectangle area, const char *rotulo, int valor, Color cor);
bool ui_alternador(Rectangle area, const char *rotulo, bool *valor);
/* Valor de 0 a 100 em passos de 10; clique nas setas ou na barra. */
bool ui_deslizador(Rectangle area, const char *rotulo, int *valor);
/* Tecla estilizada; devolve a largura ocupada. */
float ui_tecla(const char *tecla, float x, float y);
/* Tecla + ação ("E interagir"); devolve a largura ocupada com espaçamento. */
float ui_dica_tecla(const char *tecla, const char *acao, float x, float y);
/* Janela modal com título e botão de fechar; devolve a área de conteúdo e marca *fechar ao clicar no X. */
Rectangle ui_janela(const char *titulo, bool *fechar);
void ui_scanlines(Rectangle area);

#endif
