/* Cenário das telas de menu em pixel art: cidade sob chuva, torre com holofote e relâmpagos. */
#include "app.h"
#include "audio.h"
#include "pixel.h"
#include "raylib.h"
#include "render.h"
#include <math.h>

#define CICLO_RELAMPAGO_MENU 29.0f
#define HORIZONTE 196

static const Vector2 TOPO_DA_TORRE = {318, 74};

static float relampago(float tempo) {
    float fase = fmodf(tempo + 4.0f, CICLO_RELAMPAGO_MENU);
    if (fase < 0.1f) return 1.0f;
    if (fase > 0.18f && fase < 0.26f) return 0.6f;
    return 0.0f;
}

/* Trovão um pouco depois do clarão; tempo_anterior evita tocar duas vezes no mesmo ciclo. */
static void trovao(float tempo) {
    static float tempo_anterior = 0;
    float antes = fmodf(tempo_anterior + 4.0f, CICLO_RELAMPAGO_MENU), agora = fmodf(tempo + 4.0f, CICLO_RELAMPAGO_MENU);
    if (antes < 1.3f && agora >= 1.3f) audio_tocar(EFEITO_TROVAO);
    tempo_anterior = tempo;
}

/* Céu em faixas com pontilhado entre elas, mais claro perto do horizonte (luz da cidade nas nuvens). */
static void ceu(float clarao) {
    const Color topo = {8, 9, 18, 255}, base = {34, 26, 44, 255};
    for (int y = 0; y < HORIZONTE; y++) {
        float t = (float)y / HORIZONTE;
        Color linha = ColorLerp(ColorLerp(topo, base, t), (Color){150, 160, 200, 255}, clarao * 0.6f);
        px_hlinha(0, y, MUNDO_LARGURA, linha);
        if (y % 2 == 0 && px_ruido(y, 9) % 3 == 0) {
            for (int x = (int)(px_ruido(y, 1) % 7); x < MUNDO_LARGURA; x += 7) px_ponto(x, y, px_escurecer(linha, 0.8f));
        }
    }
}

static void janelas_do_predio(int x, int topo, int largura, int predio, float tempo) {
    for (int y = topo + 4; y < MUNDO_ALTURA - 4; y += 7) {
        for (int coluna = x + 3; coluna < x + largura - 3; coluna += 6) {
            unsigned ruido = px_ruido(coluna, y + predio);
            if (ruido % 9 != 0) continue;
            bool pisca = ruido % 5 == 0 && sinf(tempo * 0.7f + (float)(ruido % 13)) < -0.7f;
            if (!pisca) px_ret(coluna, y, 2, 3, ruido % 4 == 0 ? (Color){120, 170, 150, 255} : (Color){214, 160, 80, 255});
        }
    }
}

static void predios(float tempo) {
    const int alturas[] = {70, 110, 52, 132, 88, 64, 120, 58, 98, 80, 140, 72, 104};
    const int larguras[] = {34, 30, 40, 28, 36, 30, 34, 38, 30, 36, 26, 34, 32};
    int x = -6;
    for (int i = 0; i < 13; i++) {
        int topo = MUNDO_ALTURA - alturas[i];
        px_ret(x, topo, larguras[i], alturas[i], (Color){12, 13, 20, 255});
        px_hlinha(x, topo, larguras[i], (Color){30, 30, 42, 255});
        janelas_do_predio(x, topo, larguras[i], i, tempo);
        x += larguras[i] + 1;
    }
}

static void torre(float tempo) {
    int x = (int)TOPO_DA_TORRE.x, y = (int)TOPO_DA_TORRE.y;
    DrawLine(x - 14, MUNDO_ALTURA, x, y, (Color){34, 34, 46, 255});
    DrawLine(x + 14, MUNDO_ALTURA, x, y, (Color){34, 34, 46, 255});
    for (int degrau = y + 10; degrau < MUNDO_ALTURA; degrau += 12) {
        int abertura = (degrau - y) * 14 / (MUNDO_ALTURA - y);
        DrawLine(x - abertura, degrau, x + abertura, degrau + 12, (Color){28, 28, 38, 255});
    }
    px_ret(x - 3, y - 2, 7, 4, (Color){40, 40, 52, 255});
    if (sinf(tempo * 2.4f) > 0) px_ret(x - 1, y - 5, 2, 2, (Color){240, 70, 60, 255});
}

/* Holofote da torre varrendo o céu devagar. */
static void holofote(float tempo) {
    float angulo = sinf(tempo * 0.35f) * 1.1f - PI / 2;
    Vector2 ponta = {TOPO_DA_TORRE.x + cosf(angulo) * 420, TOPO_DA_TORRE.y + sinf(angulo) * 420};
    BeginBlendMode(BLEND_ADDITIVE);
    /* Dois cones sobrepostos (largo e fraco, estreito e forte) dão o miolo mais claro do feixe. */
    const float aberturas[] = {80.0f, 36.0f}, forcas[] = {0.07f, 0.10f};
    for (int camada = 0; camada < 2; camada++) {
        Vector2 lateral = {-sinf(angulo) * aberturas[camada], cosf(angulo) * aberturas[camada]};
        Vector2 a = {ponta.x + lateral.x, ponta.y + lateral.y}, b = {ponta.x - lateral.x, ponta.y - lateral.y};
        Color luz = Fade((Color){255, 246, 210, 255}, forcas[camada]);
        DrawTriangle(TOPO_DA_TORRE, a, b, luz);
        DrawTriangle(TOPO_DA_TORRE, b, a, luz); /* um dos dois sentidos é descartado pela GPU; assim um sempre aparece */
    }
    EndBlendMode();
}

static void chuva(float tempo) {
    for (int gota = 0; gota < 90; gota++) {
        unsigned ruido = px_ruido(gota, 21);
        float velocidade = 150.0f + (float)(ruido % 60);
        int x = (int)((ruido >> 4) % MUNDO_LARGURA) - (int)(fmodf(tempo * 30.0f, 40.0f));
        int y = (int)fmodf(tempo * velocidade + (float)(ruido % 400), (float)MUNDO_ALTURA + 20) - 10;
        DrawLine(x, y, x - 1, y + 4, (Color){90, 104, 140, 160});
    }
}

void telas_desenhar_fundo(float tempo) {
    float clarao = relampago(tempo);
    trovao(tempo);
    pixel_mundo_comecar(BLACK);
    ceu(clarao);
    holofote(tempo);
    torre(tempo);
    predios(tempo);
    chuva(tempo);
    pixel_vinheta(1.0f);
    pixel_mundo_terminar();
    DrawRectangleGradientH(0, 0, 700, VIRTUAL_HEIGHT, Fade(BLACK, 0.75f), Fade(BLACK, 0));
}
