#include "pixel.h"
#include "render.h"
#include <math.h>
#include <stdlib.h>

#define TAMANHO_COOKIE 64
#define NIVEIS_LUZ 6 /* poucos níveis + pontilhado = degradê com cara de pixel art */

static RenderTexture2D alvo_mundo;
static RenderTexture2D alvo_luz;
static Texture2D cookie_redondo;
static Texture2D cookie_feixe;
static Texture2D textura_vinheta;

/* Matriz de Bayer 4x4: decide quais pixels acendem em cada nível intermediário. */
static const float BAYER[4][4] = {{0, 8, 2, 10}, {12, 4, 14, 6}, {3, 11, 1, 9}, {15, 7, 13, 5}};

int px(float coordenada_virtual) { return (int)floorf(coordenada_virtual / PIXEL_ESCALA); }

float px_virtual(int pixel) { return (float)(pixel * PIXEL_ESCALA); }

static float quantizar(float valor, int x, int y) {
    float limiar = (BAYER[y % 4][x % 4] + 0.5f) / 16.0f;
    float nivel = valor * (NIVEIS_LUZ - 1);
    float base = floorf(nivel);
    return (base + (nivel - base > limiar ? 1.0f : 0.0f)) / (NIVEIS_LUZ - 1);
}

static Texture2D criar_textura(Image imagem) {
    Texture2D textura = LoadTextureFromImage(imagem);
    UnloadImage(imagem);
    SetTextureFilter(textura, TEXTURE_FILTER_POINT);
    return textura;
}

static Texture2D gerar_cookie_redondo(void) {
    Image imagem = GenImageColor(TAMANHO_COOKIE, TAMANHO_COOKIE, BLANK);
    float centro = TAMANHO_COOKIE / 2.0f;
    for (int y = 0; y < TAMANHO_COOKIE; y++) {
        for (int x = 0; x < TAMANHO_COOKIE; x++) {
            float distancia = sqrtf((x + 0.5f - centro) * (x + 0.5f - centro) + (y + 0.5f - centro) * (y + 0.5f - centro)) / centro;
            float valor = fmaxf(0.0f, 1.0f - distancia);
            unsigned char brilho = (unsigned char)(quantizar(valor * valor, x, y) * 255);
            ImageDrawPixel(&imagem, x, y, (Color){brilho, brilho, brilho, 255});
        }
    }
    return criar_textura(imagem);
}

/* Cone estreito em cima e largo embaixo, mais forte no eixo central. */
static Texture2D gerar_cookie_feixe(void) {
    const int largura = 48, altura = 96;
    Image imagem = GenImageColor(largura, altura, BLANK);
    for (int y = 0; y < altura; y++) {
        float abertura = 0.12f + 0.88f * y / altura;
        for (int x = 0; x < largura; x++) {
            float lateral = fabsf((x + 0.5f) / largura - 0.5f) * 2.0f / abertura;
            float valor = fmaxf(0.0f, 1.0f - lateral) * (1.0f - 0.45f * y / altura);
            unsigned char brilho = (unsigned char)(quantizar(fminf(1.0f, valor * 1.3f), x, y) * 255);
            ImageDrawPixel(&imagem, x, y, (Color){brilho, brilho, brilho, 255});
        }
    }
    return criar_textura(imagem);
}

/* Bordas escurecidas com pontilhado (feita uma vez; desenhar pixel a pixel por quadro seria caro). */
static Texture2D gerar_vinheta(void) {
    Image imagem = GenImageColor(MUNDO_LARGURA, MUNDO_ALTURA, BLANK);
    for (int y = 0; y < MUNDO_ALTURA; y++) {
        for (int x = 0; x < MUNDO_LARGURA; x++) {
            float dx = (x - MUNDO_LARGURA / 2.0f) / (MUNDO_LARGURA / 2.0f), dy = (y - MUNDO_ALTURA / 2.0f) / (MUNDO_ALTURA / 2.0f);
            float distancia = sqrtf(dx * dx * 0.8f + dy * dy * 0.9f);
            float valor = fminf(1.0f, fmaxf(0.0f, (distancia - 0.62f) / 0.5f));
            float pontilhado = quantizar(valor, x, y);
            if (pontilhado > 0) ImageDrawPixel(&imagem, x, y, (Color){4, 4, 10, (unsigned char)(pontilhado * 230)});
        }
    }
    return criar_textura(imagem);
}

void pixel_iniciar(void) {
    alvo_mundo = LoadRenderTexture(MUNDO_LARGURA, MUNDO_ALTURA);
    alvo_luz = LoadRenderTexture(MUNDO_LARGURA, MUNDO_ALTURA);
    SetTextureFilter(alvo_mundo.texture, TEXTURE_FILTER_POINT);
    SetTextureFilter(alvo_luz.texture, TEXTURE_FILTER_POINT);
    cookie_redondo = gerar_cookie_redondo();
    cookie_feixe = gerar_cookie_feixe();
    textura_vinheta = gerar_vinheta();
}

void pixel_encerrar(void) {
    UnloadRenderTexture(alvo_mundo);
    UnloadRenderTexture(alvo_luz);
    UnloadTexture(cookie_redondo);
    UnloadTexture(cookie_feixe);
    UnloadTexture(textura_vinheta);
}

/* ---------- luz ---------- */

void pixel_luz_comecar(Color ambiente) {
    render_pause_frame();
    BeginTextureMode(alvo_luz);
    ClearBackground(ambiente);
    BeginBlendMode(BLEND_ADDITIVE);
}

void pixel_luz(float x, float y, float raio, Color cor, float forca) {
    if (forca <= 0.0f) return;
    Rectangle origem = {0, 0, TAMANHO_COOKIE, TAMANHO_COOKIE};
    Rectangle destino = {floorf(x - raio), floorf(y - raio), floorf(raio * 2), floorf(raio * 2)};
    DrawTexturePro(cookie_redondo, origem, destino, (Vector2){0, 0}, 0.0f, Fade(cor, fminf(1.0f, forca)));
}

void pixel_feixe(Vector2 origem, Vector2 alvo, float largura, Color cor, float forca) {
    if (forca <= 0.0f) return;
    float dx = alvo.x - origem.x, dy = alvo.y - origem.y;
    float comprimento = sqrtf(dx * dx + dy * dy);
    float rotacao = atan2f(-dx, dy) * RAD2DEG;
    Rectangle fonte = {0, 0, (float)cookie_feixe.width, (float)cookie_feixe.height};
    Rectangle destino = {origem.x, origem.y, largura, comprimento};
    DrawTexturePro(cookie_feixe, fonte, destino, (Vector2){largura / 2, 0}, rotacao, Fade(cor, fminf(1.0f, forca)));
}

void pixel_luz_terminar(void) {
    EndBlendMode();
    EndTextureMode();
    render_resume_frame();
}

/* ---------- cena ---------- */

void pixel_mundo_comecar(Color fundo) {
    render_pause_frame();
    BeginTextureMode(alvo_mundo);
    ClearBackground(fundo);
}

void pixel_aplicar_luz(void) {
    BeginBlendMode(BLEND_MULTIPLIED);
    DrawTextureRec(alvo_luz.texture, (Rectangle){0, 0, MUNDO_LARGURA, -MUNDO_ALTURA}, (Vector2){0, 0}, WHITE);
    EndBlendMode();
}

void pixel_vinheta(float forca) { DrawTexture(textura_vinheta, 0, 0, Fade(WHITE, forca)); }

void pixel_mundo_terminar(void) {
    EndTextureMode();
    render_resume_frame();
    render_copy_texture(alvo_mundo.texture, (Rectangle){0, 0, MUNDO_LARGURA * PIXEL_ESCALA, MUNDO_ALTURA * PIXEL_ESCALA});
}

/* ---------- primitivas ---------- */

void px_ret(int x, int y, int largura, int altura, Color cor) { DrawRectangle(x, y, largura, altura, cor); }

void px_borda(int x, int y, int largura, int altura, Color cor) {
    DrawRectangle(x, y, largura, 1, cor);
    DrawRectangle(x, y + altura - 1, largura, 1, cor);
    DrawRectangle(x, y, 1, altura, cor);
    DrawRectangle(x + largura - 1, y, 1, altura, cor);
}

void px_hlinha(int x, int y, int largura, Color cor) { DrawRectangle(x, y, largura, 1, cor); }

void px_vlinha(int x, int y, int altura, Color cor) { DrawRectangle(x, y, 1, altura, cor); }

void px_ponto(int x, int y, Color cor) { DrawRectangle(x, y, 1, 1, cor); }

Color px_escurecer(Color cor, float fator) {
    return (Color){(unsigned char)(cor.r * fator), (unsigned char)(cor.g * fator), (unsigned char)(cor.b * fator), cor.a};
}

void px_movel(int x, int y, int largura, int altura, int altura_frente, Color tampo, Color frente) {
    const Color contorno = {14, 11, 16, 255};
    px_ret(x + 2, y + altura, largura - 1, 2, (Color){0, 0, 0, 110}); /* sombra no chão */
    px_ret(x, y, largura, altura - altura_frente, tampo);
    px_ret(x, y + altura - altura_frente, largura, altura_frente, frente);
    px_hlinha(x + 1, y + 1, largura - 2, ColorBrightness(tampo, 0.18f));
    px_hlinha(x, y + altura - altura_frente, largura, px_escurecer(tampo, 0.7f));
    px_borda(x, y, largura, altura, contorno);
}

unsigned px_ruido(int x, int y) {
    unsigned valor = (unsigned)x * 374761393u + (unsigned)y * 668265263u;
    valor = (valor ^ (valor >> 13)) * 1274126177u;
    return valor ^ (valor >> 16);
}
