#include "widgets.h"
#include "partida.h"
#include "render.h"
#include <stdio.h>

static const Rectangle JANELA = {150, 84, 980, 572};
static void (*som_clique)(void) = NULL;

void ui_definir_som_clique(void (*tocar)(void)) { som_clique = tocar; }

bool ui_mouse_sobre(Rectangle area) { return CheckCollisionPointRec(render_mouse_position(), area); }

bool ui_clicou(Rectangle area) { return IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && ui_mouse_sobre(area); }

static bool clicar_com_som(Rectangle area, bool habilitado) {
    if (!habilitado || !ui_clicou(area)) return false;
    if (som_clique) som_clique();
    return true;
}

bool ui_botao(Rectangle area, const char *rotulo, bool habilitado) {
    Color fundo = !habilitado ? (Color){26, 29, 38, 255} : ui_mouse_sobre(area) ? (Color){66, 58, 40, 255} : COLOR_CARD;
    DrawRectangleRounded(area, 0.2f, 6, fundo);
    DrawRectangleRoundedLinesEx(area, 0.2f, 6, 1.5f, habilitado ? Fade(COLOR_EDGE, 0.7f) : Fade(COLOR_MUTED, 0.2f));
    float tamanho = area.height < 36 ? 14 : 15;
    render_text_centered(rotulo, area.x + area.width / 2, area.y + (area.height - tamanho) / 2 - 1, tamanho,
                         habilitado ? COLOR_CREAM : Fade(COLOR_MUTED, 0.6f));
    return clicar_com_som(area, habilitado);
}

/* Só ações que gastam bateria mostram etiqueta; o que é de graça fica limpo. */
static void desenhar_custo(Rectangle area, int custo, bool habilitado) {
    if (custo <= 0) return;
    /* Cada ação consome 15 minutos de bateria; mostrar o tempo é mais natural do que "pontos". */
    const char *texto = TextFormat("%d min", custo * MINUTOS_POR_ACAO);
    float largura = render_text_width(texto, 12) + 14;
    Rectangle etiqueta = {area.x + area.width - largura - 8, area.y + (area.height - 20) / 2, largura, 20};
    Color cor = custo == 0 ? COLOR_SIGNAL : COLOR_AMBER;
    DrawRectangleRounded(etiqueta, 0.5f, 4, Fade(cor, habilitado ? 0.2f : 0.08f));
    render_text_centered(texto, etiqueta.x + largura / 2, etiqueta.y + 3, 12, habilitado ? cor : Fade(cor, 0.4f));
}

bool ui_botao_custo(Rectangle area, const char *rotulo, int custo, bool habilitado) {
    Color fundo = !habilitado ? (Color){26, 29, 38, 255} : ui_mouse_sobre(area) ? (Color){66, 58, 40, 255} : COLOR_CARD;
    DrawRectangleRounded(area, 0.2f, 6, fundo);
    DrawRectangleRoundedLinesEx(area, 0.2f, 6, 1.5f, habilitado ? Fade(COLOR_EDGE, 0.7f) : Fade(COLOR_MUTED, 0.2f));
    render_text(rotulo, area.x + 12, area.y + (area.height - 15) / 2 - 1, 15, habilitado ? COLOR_CREAM : Fade(COLOR_MUTED, 0.6f));
    desenhar_custo(area, custo, habilitado);
    return clicar_com_som(area, habilitado);
}

bool ui_botao_principal(Rectangle area, const char *rotulo, int custo, bool habilitado) {
    Color fundo = !habilitado ? (Color){40, 34, 30, 255} : ui_mouse_sobre(area) ? (Color){150, 104, 50, 255} : (Color){120, 82, 40, 255};
    DrawRectangleRounded(area, 0.2f, 6, fundo);
    DrawRectangleRoundedLinesEx(area, 0.2f, 6, 2, habilitado ? COLOR_AMBER : Fade(COLOR_MUTED, 0.3f));
    render_text(rotulo, area.x + 16, area.y + (area.height - 18) / 2 - 1, 18, habilitado ? COLOR_CREAM : Fade(COLOR_MUTED, 0.6f));
    if (custo >= 0) desenhar_custo(area, custo, habilitado);
    return clicar_com_som(area, habilitado);
}

bool ui_item_lista(Rectangle area, const char *titulo, const char *detalhe, bool selecionado, Color cor_detalhe) {
    Color fundo = selecionado ? COLOR_CARD_SELECTED : ui_mouse_sobre(area) ? (Color){42, 48, 62, 255} : COLOR_CARD;
    DrawRectangleRounded(area, 0.15f, 6, fundo);
    if (selecionado) DrawRectangleRoundedLinesEx(area, 0.15f, 6, 2, COLOR_AMBER);
    bool tem_detalhe = detalhe && detalhe[0];
    float y_titulo = tem_detalhe ? area.y + (area.height >= 46 ? 7 : 4) : area.y + (area.height - 15) / 2 - 1;
    render_text(titulo, area.x + 12, y_titulo, 15, COLOR_CREAM);
    if (tem_detalhe) render_text(detalhe, area.x + 12, area.y + area.height - 18, 12, cor_detalhe);
    return clicar_com_som(area, true);
}

void ui_etiqueta(float x, float y, const char *texto, Color cor) {
    float largura = render_text_width(texto, 12) + 16;
    Rectangle area = {x, y, largura, 22};
    DrawRectangleRounded(area, 0.5f, 4, Fade(cor, 0.18f));
    DrawRectangleRoundedLinesEx(area, 0.5f, 4, 1.2f, cor);
    render_text_centered(texto, x + largura / 2, y + 4, 12, cor);
}

void ui_medidor(Rectangle area, const char *rotulo, int valor, Color cor) {
    render_text(rotulo, area.x, area.y, 11, COLOR_MUTED);
    render_text(TextFormat("%d%%", valor), area.x + area.width - render_text_width(TextFormat("%d%%", valor), 12), area.y - 1, 12, cor);
    Rectangle barra = {area.x, area.y + 16, area.width, area.height - 16};
    DrawRectangleRounded(barra, 0.5f, 4, (Color){30, 33, 44, 255});
    float preenchido = barra.width * (valor < 0 ? 0 : valor > 100 ? 100 : valor) / 100.0f;
    if (preenchido > 1) DrawRectangleRounded((Rectangle){barra.x, barra.y, preenchido, barra.height}, 0.5f, 4, cor);
}

bool ui_alternador(Rectangle area, const char *rotulo, bool *valor) {
    bool sobre = ui_mouse_sobre(area);
    DrawRectangleRounded(area, 0.2f, 6, sobre ? (Color){42, 48, 62, 255} : COLOR_CARD);
    render_text(rotulo, area.x + 16, area.y + (area.height - 17) / 2, 17, COLOR_CREAM);
    Rectangle trilho = {area.x + area.width - 70, area.y + (area.height - 24) / 2, 52, 24};
    DrawRectangleRounded(trilho, 1.0f, 8, *valor ? Fade(COLOR_SIGNAL, 0.5f) : (Color){50, 54, 66, 255});
    float bola_x = *valor ? trilho.x + trilho.width - 12 : trilho.x + 12;
    DrawCircle((int)bola_x, (int)(trilho.y + 12), 9, *valor ? COLOR_SIGNAL : COLOR_MUTED);
    if (!clicar_com_som(area, true)) return false;
    *valor = !*valor;
    return true;
}

static int limitar_volume(int valor) { return valor < 0 ? 0 : valor > 100 ? 100 : valor; }

bool ui_deslizador(Rectangle area, const char *rotulo, int *valor) {
    DrawRectangleRounded(area, 0.2f, 6, ui_mouse_sobre(area) ? (Color){42, 48, 62, 255} : COLOR_CARD);
    render_text(rotulo, area.x + 16, area.y + (area.height - 17) / 2, 17, COLOR_CREAM);
    Rectangle menos = {area.x + area.width - 300, area.y + 8, 34, area.height - 16};
    Rectangle mais = {area.x + area.width - 52, area.y + 8, 34, area.height - 16};
    Rectangle barra = {menos.x + 44, area.y + area.height / 2 - 4, mais.x - menos.x - 88, 8};
    DrawRectangleRounded(barra, 1.0f, 4, (Color){50, 54, 66, 255});
    DrawRectangleRounded((Rectangle){barra.x, barra.y, barra.width * *valor / 100.0f, barra.height}, 1.0f, 4, COLOR_AMBER);
    render_text(TextFormat("%d", *valor), barra.x + barra.width + 8, area.y + (area.height - 14) / 2, 14, COLOR_MUTED);
    int antes = *valor;
    if (ui_botao(menos, "-", *valor > 0)) *valor = limitar_volume(*valor - 10);
    if (ui_botao(mais, "+", *valor < 100)) *valor = limitar_volume(*valor + 10);
    Rectangle area_clique = {barra.x, barra.y - 8, barra.width, barra.height + 16};
    if (ui_clicou(area_clique)) *valor = limitar_volume((int)((render_mouse_position().x - barra.x) / barra.width * 10 + 0.5f) * 10);
    return *valor != antes;
}

float ui_tecla(const char *tecla, float x, float y) {
    float largura = render_text_width(tecla, 14) + 14;
    Rectangle tampa = {x, y, largura, 22};
    DrawRectangleRounded((Rectangle){tampa.x, tampa.y + 2, tampa.width, tampa.height}, 0.3f, 4, (Color){8, 9, 14, 255});
    DrawRectangleRounded(tampa, 0.3f, 4, (Color){52, 56, 68, 255});
    render_text(tecla, x + 7, y + 3, 14, COLOR_CREAM);
    return largura;
}

float ui_dica_tecla(const char *tecla, const char *acao, float x, float y) {
    float largura_tecla = ui_tecla(tecla, x, y);
    render_text(acao, x + largura_tecla + 8, y + 3, 14, COLOR_MUTED);
    return largura_tecla + 8 + render_text_width(acao, 14) + 24;
}

Rectangle ui_janela(const char *titulo, bool *fechar) {
    DrawRectangle(0, 0, VIRTUAL_WIDTH, VIRTUAL_HEIGHT, Fade(BLACK, 0.6f));
    DrawRectangleRec((Rectangle){JANELA.x + 10, JANELA.y + 12, JANELA.width, JANELA.height}, Fade(BLACK, 0.5f));
    DrawRectangleRec(JANELA, COLOR_PANEL);
    DrawRectangle((int)JANELA.x, (int)JANELA.y, (int)JANELA.width, 56, COLOR_BAR);
    DrawRectangleLinesEx(JANELA, 3, COLOR_EDGE);
    DrawLine((int)JANELA.x, (int)JANELA.y + 56, (int)(JANELA.x + JANELA.width), (int)JANELA.y + 56, Fade(COLOR_EDGE, 0.5f));
    DrawCircle((int)JANELA.x + 30, (int)JANELA.y + 28, 6, COLOR_ALERT);
    render_text(titulo, JANELA.x + 48, JANELA.y + 15, 24, COLOR_CREAM);
    Rectangle botao = {JANELA.x + JANELA.width - 134, JANELA.y + 13, 114, 30};
    if (ui_botao(botao, "ESC  fechar", true) && fechar) *fechar = true;
    /* A faixa de baixo (y 598 a 653) fica livre para o aviso do resultado das ações. */
    DrawRectangle((int)JANELA.x + 3, 598, (int)JANELA.width - 6, 55, COLOR_BAR);
    return (Rectangle){JANELA.x + 24, JANELA.y + 70, JANELA.width - 48, 436};
}

void ui_scanlines(Rectangle area) {
    for (float y = area.y; y < area.y + area.height; y += 3) {
        DrawLineV((Vector2){area.x, y}, (Vector2){area.x + area.width, y}, Fade(BLACK, 0.12f));
    }
}
