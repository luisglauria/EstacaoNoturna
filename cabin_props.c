/* Móveis, equipamentos e objetos em pixel art. Cada objeto tem a passada "cena" (afetada pela luz)
   e a passada "brilho" (telas, LEDs e lâmpadas, desenhados depois da luz para acenderem no escuro). */
#include "cabin_internal.h"
#include "sprites.h"
#include <math.h>

/* ---------- peças pequenas ---------- */

static void papel(int x, int y, int largura, int altura, int linhas) {
    px_ret(x + 1, y + 1, largura, altura, (Color){0, 0, 0, 90});
    px_ret(x, y, largura, altura, COR_PAPEL);
    for (int linha = 0; linha < linhas; linha++) px_hlinha(x + 1, y + 2 + linha * 2, largura - 2 - (linha % 2) * 2, (Color){150, 140, 120, 255});
}

static void caneca(int x, int y) {
    px_ret(x, y, 4, 4, (Color){180, 70, 60, 255});
    px_hlinha(x, y, 4, (Color){40, 26, 22, 255});
    px_ponto(x + 4, y + 1, (Color){180, 70, 60, 255});
}

void cabin_draw_rug(void) {
    const int x = 166, y = 98, largura = 94, altura = 56;
    px_ret(x, y, largura, altura, (Color){112, 34, 40, 255});
    px_borda(x + 3, y + 3, largura - 6, altura - 6, (Color){170, 120, 66, 255});
    px_borda(x + 6, y + 6, largura - 12, altura - 12, (Color){84, 24, 30, 255});
    for (int i = 0; i < 9; i++) {
        int centro_x = x + largura / 2, centro_y = y + altura / 2;
        px_hlinha(centro_x - i, centro_y - 9 + i, i * 2 + 1, (Color){140, 54, 50, 255});
        px_hlinha(centro_x - i, centro_y + 9 - i, i * 2 + 1, (Color){140, 54, 50, 255});
    }
    for (int franja = x; franja < x + largura; franja += 2) {
        px_ponto(franja, y - 1, (Color){190, 170, 130, 255});
        px_ponto(franja, y + altura, (Color){190, 170, 130, 255});
    }
}

void cabin_draw_chair(void) {
    px_movel(207, 166, 13, 12, 3, (Color){96, 66, 46, 255}, (Color){62, 42, 30, 255});
    px_ret(207, 164, 13, 3, (Color){70, 48, 34, 255}); /* encosto */
    px_borda(207, 164, 13, 3, COR_CONTORNO);
}

/* ---------- equipamentos: passada da cena ---------- */

static void receptor(PxRect r) {
    px_movel(r.x, r.y, r.w, r.h, 5, COR_MADEIRA, COR_MADEIRA_FRENTE);
    px_movel(r.x + 4, r.y + 2, 40, 16, 4, (Color){76, 92, 96, 255}, (Color){50, 62, 66, 255});
    px_ret(r.x + 8, r.y + 4, 20, 7, (Color){10, 18, 16, 255});
    DrawCircle(r.x + 36, r.y + 8, 4, (Color){44, 52, 56, 255});
    DrawCircleLines(r.x + 36, r.y + 8, 4, (Color){190, 150, 80, 255});
    DrawLine(r.x + 42, r.y + 2, r.x + 50, r.y - 4, (Color){150, 150, 156, 255}); /* antena */
    papel(r.x + 50, r.y + 4, 11, 13, 5);
    caneca(r.x + 64, r.y + 7);
}

static void transmissor(PxRect r) {
    px_movel(r.x, r.y, r.w, r.h, 5, COR_MADEIRA, COR_MADEIRA_FRENTE);
    px_movel(r.x + 4, r.y + 1, 46, 19, 5, (Color){112, 58, 64, 255}, (Color){74, 36, 42, 255});
    px_ret(r.x + 8, r.y + 4, 24, 8, (Color){16, 10, 14, 255});
    for (int botao = 0; botao < 4; botao++) px_ret(r.x + 9 + botao * 6, r.y + 16, 3, 2, (Color){40, 28, 32, 255});
    px_vlinha(r.x + 60, r.y + 8, 12, (Color){60, 60, 66, 255}); /* pedestal do microfone */
    px_ret(r.x + 57, r.y + 20, 7, 2, (Color){50, 50, 56, 255});
    DrawCircle(r.x + 60, r.y + 6, 3, (Color){52, 52, 60, 255});
    px_ponto(r.x + 59, r.y + 5, (Color){150, 150, 160, 255});
}

static void bancada(PxRect r, bool provas_escondidas) {
    px_movel(r.x, r.y, r.w, r.h, 6, (Color){108, 78, 54, 255}, (Color){70, 48, 34, 255});
    if (!provas_escondidas) {
        papel(r.x + 8, r.y + 4, 14, 16, 6);
        papel(r.x + 26, r.y + 2, 13, 15, 6);
        papel(r.x + 46, r.y + 6, 15, 11, 4);
        papel(r.x + 66, r.y + 3, 12, 16, 6);
        DrawLine(r.x + 15, r.y + 6, r.x + 52, r.y + 9, (Color){190, 50, 50, 255}); /* fio vermelho ligando pistas */
        DrawLine(r.x + 52, r.y + 9, r.x + 72, r.y + 5, (Color){190, 50, 50, 255});
    } else {
        px_borda(r.x - 16, r.y + 20, 12, 8, (Color){40, 28, 22, 255}); /* tábua solta do fundo falso */
    }
    DrawCircleLines(r.x + 40, r.y + 19, 3, (Color){200, 200, 190, 255}); /* lupa */
    px_ret(r.x + 96, r.y + 6, 8, 3, (Color){52, 60, 52, 255});           /* abajur */
    px_vlinha(r.x + 100, r.y + 3, 4, (Color){40, 40, 40, 255});
}

static void painel_frequencia(PxRect r) {
    px_movel(r.x, r.y, r.w, r.h, r.h - 6, (Color){66, 88, 84, 255}, (Color){44, 62, 58, 255});
    px_ret(r.x + 3, r.y + 9, 14, 10, (Color){8, 18, 14, 255});
    for (int linha = 0; linha < 5; linha++) {
        DrawCircle(r.x + 6, r.y + 26 + linha * 7, 2, (Color){30, 40, 38, 255});
        DrawCircle(r.x + 14, r.y + 26 + linha * 7, 2, (Color){30, 40, 38, 255});
    }
}

static void colchao(PxRect r) {
    px_movel(r.x, r.y, r.w, r.h, 3, (Color){176, 166, 146, 255}, (Color){120, 112, 98, 255});
    px_ret(r.x + 4, r.y + 3, r.w - 8, 9, (Color){214, 206, 190, 255});
    px_borda(r.x + 4, r.y + 3, r.w - 8, 9, (Color){150, 140, 124, 255});
    px_ret(r.x + 1, r.y + 18, r.w - 2, r.h - 22, (Color){52, 80, 72, 255});
    px_hlinha(r.x + 1, r.y + 18, r.w - 2, (Color){76, 110, 98, 255});
    for (int dobra = r.y + 26; dobra < r.y + r.h - 6; dobra += 9) px_hlinha(r.x + 6, dobra, r.w - 14, (Color){40, 64, 56, 255});
}

static void arquivo(PxRect r) {
    px_movel(r.x, r.y, r.w, r.h, r.h - 5, (Color){92, 102, 110, 255}, (Color){66, 74, 82, 255});
    for (int gaveta = 0; gaveta < 3; gaveta++) {
        int y = r.y + 7 + gaveta * 9;
        px_borda(r.x + 2, y, r.w - 4, 8, (Color){44, 50, 56, 255});
        px_hlinha(r.x + 7, y + 3, 4, (Color){180, 180, 170, 255});
    }
    px_ret(r.x + 12, r.y + 15, 4, 3, COR_PAPEL); /* ponta do bilhete escapando da gaveta */
}

void cabin_draw_spot(const EquipmentSpot *spot, float time, const CabinScene *scene) {
    (void)time;
    switch (spot->id) {
        case EQUIPMENT_RECEIVER: receptor(spot->body); break;
        case EQUIPMENT_TRANSMITTER: transmissor(spot->body); break;
        case EQUIPMENT_DESK: bancada(spot->body, scene->evidence_hidden); break;
        case EQUIPMENT_FREQUENCY: painel_frequencia(spot->body); break;
        case EQUIPMENT_MATTRESS: colchao(spot->body); break;
        case EQUIPMENT_NOTE: arquivo(spot->body); break;
        case EQUIPMENT_HOME_RADIO: sprite_objeto(SPRITE_RADIO_DOMESTICO, spot->body.x, spot->body.y); break;
        default: break; /* janela, porta, foto e marcas ficam com as paredes */
    }
}

/* ---------- passada do brilho ---------- */

static void onda(int x, int y, int largura, int altura, float time, float velocidade, Color cor) {
    for (int coluna = 0; coluna < largura; coluna++) {
        float valor = sinf(coluna * 0.7f + time * velocidade) * sinf(coluna * 0.13f + time * 1.1f);
        px_ponto(x + coluna, y + altura / 2 + (int)roundf(valor * (altura / 2 - 1)), cor);
    }
}

static void brilho_receptor(PxRect r, float time) {
    onda(r.x + 9, r.y + 4, 18, 7, time, 5.0f, COR_TELA_VERDE);
    if (fmodf(time, 1.4f) < 0.7f) px_ponto(r.x + 7, r.y + 14, COR_TELA_VERDE);
    float angulo = time * 0.6f;
    px_ponto(r.x + 36 + (int)roundf(cosf(angulo) * 3), r.y + 8 + (int)roundf(sinf(angulo) * 3), COR_AMBAR);
}

static void brilho_transmissor(PxRect r, float time) {
    for (int barra = 0; barra < 6; barra++) {
        int nivel = 2 + (int)(5 * fabsf(sinf(time * (2.1f + barra * 0.7f) + barra)));
        px_ret(r.x + 9 + barra * 4, r.y + 11 - nivel, 3, nivel, nivel > 5 ? COR_VERMELHA : COR_AMBAR);
    }
    if (sinf(time * 4.0f) > -0.2f) px_ret(r.x + 38, r.y + 5, 3, 3, COR_VERMELHA);
}

static void brilho_painel(PxRect r, float time) {
    onda(r.x + 4, r.y + 10, 12, 8, time, 7.0f, COR_TELA_VERDE);
    for (int led = 0; led < 3; led++) {
        if (sinf(time * (1.5f + led)) > 0) px_ponto(r.x + 16, r.y + 26 + led * 7, COR_TELA_VERDE);
    }
}

void cabin_draw_spot_glow(const EquipmentSpot *spot, float time, const CabinScene *scene) {
    if (spot->id == EQUIPMENT_DESK) {
        px_ret(spot->body.x + 98, spot->body.y + 9, 4, 1, (Color){255, 230, 170, 255}); /* lâmpada do abajur */
        return;
    }
    if (!scene->equipment_on) return;
    if (spot->id == EQUIPMENT_RECEIVER) brilho_receptor(spot->body, time);
    if (spot->id == EQUIPMENT_TRANSMITTER) brilho_transmissor(spot->body, time);
    if (spot->id == EQUIPMENT_FREQUENCY) brilho_painel(spot->body, time);
    if (spot->id == EQUIPMENT_HOME_RADIO && fmodf(time, 2.0f) < 1.0f) px_ponto(spot->body.x + 9, spot->body.y + 6, COR_AMBAR);
}
