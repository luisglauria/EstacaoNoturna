#include "interface.h"
#include "rlgl.h"
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ---------- tela virtual e texto ---------- */
static const char *FONT_PATHS[] = {
    "assets/font.ttf",
    "C:/Windows/Fonts/consola.ttf",
    "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",
};
#define FONT_BASE_SIZE 48
#define FONT_FIRST_CODEPOINT 32
#define FONT_CODEPOINT_COUNT 224

static Font game_font;
static bool has_custom_font = false;
static RenderTexture2D frame_target;

static bool try_load_font(const char *path) {
    if (!FileExists(path)) return false;
    int codepoints[FONT_CODEPOINT_COUNT];
    for (int i = 0; i < FONT_CODEPOINT_COUNT; i++) codepoints[i] = FONT_FIRST_CODEPOINT + i;
    Font font = LoadFontEx(path, FONT_BASE_SIZE, codepoints, FONT_CODEPOINT_COUNT);
    if (font.texture.id == GetFontDefault().texture.id) return false;
    SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR);
    game_font = font;
    return true;
}

void render_init(void) {
    game_font = GetFontDefault();
    for (size_t i = 0; i < sizeof(FONT_PATHS) / sizeof(FONT_PATHS[0]) && !has_custom_font; i++) {
        has_custom_font = try_load_font(FONT_PATHS[i]);
    }
    frame_target = LoadRenderTexture(VIRTUAL_WIDTH, VIRTUAL_HEIGHT);
    SetTextureFilter(frame_target.texture, TEXTURE_FILTER_BILINEAR);
}

void render_shutdown(void) {
    UnloadRenderTexture(frame_target);
    if (has_custom_font) UnloadFont(game_font);
}

static float frame_scale(void) {
    float scale_x = (float)GetScreenWidth() / VIRTUAL_WIDTH;
    float scale_y = (float)GetScreenHeight() / VIRTUAL_HEIGHT;
    return fminf(scale_x, scale_y);
}

static Vector2 frame_offset(float scale) {
    return (Vector2){(GetScreenWidth() - VIRTUAL_WIDTH * scale) * 0.5f,
                     (GetScreenHeight() - VIRTUAL_HEIGHT * scale) * 0.5f};
}

void render_begin_frame(void) { BeginTextureMode(frame_target); }

static void render_pause_frame(void) { EndTextureMode(); }

static void render_resume_frame(void) { BeginTextureMode(frame_target); }

static void render_copy_texture(Texture2D texture, Rectangle dest) {
    Rectangle source = {0, 0, (float)texture.width, (float)-texture.height};
    rlSetBlendFactorsSeparate(RL_ONE, RL_ZERO, RL_ZERO, RL_ONE, RL_FUNC_ADD, RL_FUNC_ADD);
    BeginBlendMode(BLEND_CUSTOM_SEPARATE);
    DrawTexturePro(texture, source, dest, (Vector2){0, 0}, 0.0f, WHITE);
    EndBlendMode();
}

/* Copia a tela virtual para a janela, escalando e centralizando. */
void render_end_frame(void) {
    EndTextureMode();
    float scale = frame_scale();
    Vector2 offset = frame_offset(scale);
    BeginDrawing();
    ClearBackground(BLACK);
    render_copy_texture(frame_target.texture, (Rectangle){offset.x, offset.y, VIRTUAL_WIDTH * scale, VIRTUAL_HEIGHT * scale});
    EndDrawing();
}

static Vector2 render_mouse_position(void) {
    float scale = frame_scale();
    Vector2 offset = frame_offset(scale);
    Vector2 mouse = GetMousePosition();
    return (Vector2){(mouse.x - offset.x) / scale, (mouse.y - offset.y) / scale};
}

static float text_spacing(float size) { return has_custom_font ? 0.0f : size / 10.0f; }

void render_text(const char *text, float x, float y, float size, Color color) {
    DrawTextEx(game_font, text, (Vector2){roundf(x), roundf(y)}, size, text_spacing(size), color);
}

float render_text_width(const char *text, float size) {
    return MeasureTextEx(game_font, text, size, text_spacing(size)).x;
}

void render_text_centered(const char *text, float center_x, float y, float size, Color color) {
    render_text(text, center_x - render_text_width(text, size) * 0.5f, y, size, color);
}

#define WRAP_LINE_MAX 512

static const char *next_word(const char *cursor, char *word, size_t word_size) {
    const char *start = cursor;
    while (*cursor && *cursor != ' ' && *cursor != '\n') cursor++;
    size_t length = (size_t)(cursor - start);
    if (length >= word_size) length = word_size - 1;
    memcpy(word, start, length);
    word[length] = '\0';
    return cursor;
}

static void copy_text(char *destination, size_t size, const char *source) {
    size_t length = strlen(source);
    if (length >= size) length = size - 1;
    memcpy(destination, source, length);
    destination[length] = '\0';
}

static void flush_line(char *line, Rectangle area, float *y, float size, Color color, bool draw) {
    if (draw && line[0]) render_text(line, area.x, *y, size, color);
    *y += size * 1.35f;
    line[0] = '\0';
}

/* Quebra o texto por palavras dentro da largura; devolve a altura usada. */
static float layout_wrapped(const char *text, Rectangle area, float size, Color color, bool draw) {
    char line[WRAP_LINE_MAX] = "", word[WRAP_LINE_MAX], attempt[WRAP_LINE_MAX * 2];
    float y = area.y;
    const char *cursor = text;
    while (*cursor) {
        cursor = next_word(cursor, word, sizeof(word));
        snprintf(attempt, sizeof(attempt), "%s%s%s", line, line[0] ? " " : "", word);
        if (line[0] && render_text_width(attempt, size) > area.width) {
            flush_line(line, area, &y, size, color, draw);
            copy_text(line, sizeof(line), word);
        } else {
            copy_text(line, sizeof(line), attempt);
        }
        if (*cursor == '\n') flush_line(line, area, &y, size, color, draw);
        if (*cursor) cursor++;
    }
    if (line[0]) flush_line(line, area, &y, size, color, draw);
    return y - area.y;
}

float render_text_wrapped(const char *text, Rectangle area, float size, Color color) {
    return layout_wrapped(text, area, size, color, true);
}

float render_text_wrapped_height(const char *text, float width, float size) {
    return layout_wrapped(text, (Rectangle){0, 0, width, 0}, size, BLANK, false);
}

/* ---------- pixel art e luz ---------- */
#define TAMANHO_COOKIE 64
#define NIVEIS_LUZ 6

static RenderTexture2D alvo_mundo;
static RenderTexture2D alvo_luz;
static Texture2D cookie_redondo;
static Texture2D cookie_feixe;
static Texture2D textura_vinheta;

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

void pixel_mundo_comecar(Color fundo) {
    render_pause_frame();
    BeginTextureMode(alvo_mundo);
    ClearBackground(fundo);
}

/* Multiplica o mapa de luz sobre a cena: fora das luzes fica escuro. */
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
    px_ret(x + 2, y + altura, largura - 1, 2, (Color){0, 0, 0, 110});
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

/* ---------- sprites: cada letra do mapa é uma cor ---------- */
#define OPERADOR_LARGURA 16
#define OPERADOR_ALTURA 24
#define LINHAS_TRONCO 18
#define LINHAS_PERNAS 6
#define QUADROS 3

static Color cor_da_letra(char letra) {
    switch (letra) {
        case 'K': return (Color){18, 14, 22, 255};
        case 'H': return (Color){58, 40, 34, 255};
        case 'h': return (Color){92, 64, 48, 255};
        case 'S': return (Color){226, 176, 134, 255};
        case 's': return (Color){178, 128, 98, 255};
        case 'E': return (Color){24, 18, 26, 255};
        case 'F': return (Color){34, 34, 42, 255};
        case 'f': return (Color){96, 96, 110, 255};
        case 'R': return (Color){160, 44, 50, 255};
        case 'r': return (Color){112, 30, 38, 255};
        case 'C': return (Color){62, 74, 100, 255};
        case 'c': return (Color){40, 48, 68, 255};
        case 'L': return (Color){92, 106, 140, 255};
        case 'P': return (Color){44, 44, 58, 255};
        case 'p': return (Color){28, 28, 38, 255};
        case 'B': return (Color){44, 32, 28, 255};
        case 'b': return (Color){78, 58, 46, 255};
        case 'W': return (Color){150, 110, 70, 255};
        case 'A': return (Color){200, 180, 142, 255};
        case 'a': return (Color){116, 96, 76, 255};
        case 'M': return (Color){120, 76, 44, 255};
        case 'm': return (Color){82, 52, 32, 255};
        case 'G': return (Color){62, 50, 42, 255};
        case 'g': return (Color){38, 30, 26, 255};
        case 'O': return (Color){214, 182, 110, 255};
        case 'o': return (Color){150, 118, 70, 255};
        default: return BLANK;
    }
}

static const char *const TRONCO_FRENTE[LINHAS_TRONCO] = {
    "......KKKK......", "....KKFFFFKK....", "...KFHHHHHHFK...", "..KFHHhhhhHHFK..", "..KFHHHHHHHHFK..",
    ".KffKHSSSSHKffK.", ".KffKSESSESKffK.", ".KFFKSSSSSSKFFK.", "..KK.KSssSK.KK..", "....KRRRRRRK....",
    "...KCRrRRrRCK...", "..KCLCRRRRCLCK..", ".KCLCCCCCCCCLCK.", ".KCLCCCKKCCCLCK.", ".KcLCCCKKCCCLcK.",
    ".KSKcCCKKCCcKSK.", ".KKKccccccccKKK.", "...KPPPPPPPPK...",
};

static const char *const TRONCO_COSTAS[LINHAS_TRONCO] = {
    "......KKKK......", "....KKFFFFKK....", "...KFHHHHHHFK...", "..KFHHHHHHHHFK..", "..KFHHhHHhHHFK..",
    ".KffKHHHHHHKffK.", ".KffKHHhhHHKffK.", ".KFFKHHHHHHKFFK.", "..KK.KsSSsK.KK..", "....KRRRRRRK....",
    "...KCRRRRRRCK...", "..KCLCCCCCCLCK..", ".KCLCCCCCCCCLCK.", ".KCLCCCcCCCCLCK.", ".KcLCCCcCCCCLcK.",
    ".KSKcCCcCCCcKSK.", ".KKKccccccccKKK.", "...KPPPPPPPPK...",
};

static const char *const TRONCO_LADO[LINHAS_TRONCO] = {
    ".....KKKKK......", "....KHHHHFK.....", "...KHHhhHFK.....", "..KHHHHHHFFK....", "..KSHHHHHffK....",
    ".KSSSHHHHffK....", ".KESSSHHHFFK....", "..KSSSSHHHK.....", "...KKsSSKK......", "....KRRRRK......",
    "...KRRrCCCK.....", "...KCCCCLCK.....", "...KCCCCLCK.....", "...KcCCCLCK.....", "...KcCSSLCK.....",
    "...KcCSScCK.....", "...KKcccccK.....", "....KPPPPK......",
};

static const char *const PERNAS_FRENTE[QUADROS][LINHAS_PERNAS] = {
    {"...KPPPKKPPPK...", "...KPPPKKPPPK...", "...KpPPKKPPpK...", "...KPPK..KPPK...", "..KBBbK..KbBBK..", "..KKKKK..KKKKK.."},
    {"...KPPPKKPPPK...", "...KpPPKKPPPK...", "...KBBbKKPPpK...", "...KKKKK.KPPK...", ".........KbBBK..", ".........KKKKK.."},
    {"...KPPPKKPPPK...", "...KPPPKKPPpK...", "...KpPPKKbBBK...", "...KPPK.KKKKK...", "..KBBbK.........", "..KKKKK........."},
};

static const char *const PERNAS_LADO[QUADROS][LINHAS_PERNAS] = {
    {"....KPPPPK......", "....KPPPpK......", "....KPPPpK......", "....KPPPPK......", "...KBBBBbK......", "...KKKKKKK......"},
    {"....KPPPPK......", "...KPPKKPPK.....", "..KPPK..KPPK....", "..KPK....KPK....", ".KBBK....KBbK...", ".KKKK....KKKK..."},
    {"....KPPPPK......", "....KPPPPK......", "...KPPKKPPK.....", "...KPK..KPK.....", "..KBbK..KBBK....", "..KKKK..KKKK...."},
};

static Texture2D operador[3][QUADROS];

static void pintar_linhas(Image *imagem, const char *const linhas[], int total, int y_inicial) {
    for (int linha = 0; linha < total; linha++) {
        int largura = (int)strlen(linhas[linha]);
        for (int coluna = 0; coluna < largura; coluna++) {
            Color cor = cor_da_letra(linhas[linha][coluna]);
            if (cor.a) ImageDrawPixel(imagem, coluna, y_inicial + linha, cor);
        }
    }
}

static Texture2D montar_operador(const char *const tronco[], const char *const pernas[]) {
    Image imagem = GenImageColor(OPERADOR_LARGURA, OPERADOR_ALTURA, BLANK);
    pintar_linhas(&imagem, tronco, LINHAS_TRONCO, 0);
    pintar_linhas(&imagem, pernas, LINHAS_PERNAS, LINHAS_TRONCO);
    Texture2D textura = LoadTextureFromImage(imagem);
    UnloadImage(imagem);
    return textura;
}

static const char *const FOTO[] = {
    "KKKKKKKK", "KWWWWWWK", "KWAAAAWK", "KWAaaAWK", "KWAaaAWK", "KWAAaAWK", "KWWWWWWK", "KKKKKKKK",
};

static const char *const RADIO_DOMESTICO[] = {
    "..KKKKKKKK..", ".KMMMMMMMMK.", "KMmGGGGmOOMK", "KMmGgGgmOoMK", "KMmGGGGmOOMK", "KMmmmmmmmmMK", "KMMMMMMMMMMK", ".KKKKKKKKKK.",
};

static Texture2D objetos[SPRITE_TOTAL];

static Texture2D montar_mapa(const char *const linhas[], int total) {
    Image imagem = GenImageColor((int)strlen(linhas[0]), total, BLANK);
    pintar_linhas(&imagem, linhas, total, 0);
    Texture2D textura = LoadTextureFromImage(imagem);
    UnloadImage(imagem);
    return textura;
}

void sprites_iniciar(void) {
    for (int quadro = 0; quadro < QUADROS; quadro++) {
        operador[0][quadro] = montar_operador(TRONCO_FRENTE, PERNAS_FRENTE[quadro]);
        operador[1][quadro] = montar_operador(TRONCO_COSTAS, PERNAS_FRENTE[quadro]);
        operador[2][quadro] = montar_operador(TRONCO_LADO, PERNAS_LADO[quadro]);
    }
    objetos[SPRITE_FOTO] = montar_mapa(FOTO, 8);
    objetos[SPRITE_RADIO_DOMESTICO] = montar_mapa(RADIO_DOMESTICO, 8);
}

void sprites_encerrar(void) {
    for (int vista = 0; vista < 3; vista++) {
        for (int quadro = 0; quadro < QUADROS; quadro++) UnloadTexture(operador[vista][quadro]);
    }
    for (int i = 0; i < SPRITE_TOTAL; i++) UnloadTexture(objetos[i]);
}

static int quadro_do_passo(int passo, bool lado) {
    if (passo == 1) return 1;
    if (passo == 3) return lado ? 1 : 2;
    return passo == 2 && lado ? 2 : 0;
}

void sprite_operador(int x, int y_pes, Direcao direcao, int passo) {
    bool lado = direcao == DIRECAO_ESQUERDA || direcao == DIRECAO_DIREITA;
    int vista = direcao == DIRECAO_BAIXO ? 0 : direcao == DIRECAO_CIMA ? 1 : 2;
    Texture2D textura = operador[vista][quadro_do_passo(passo, lado)];
    float largura = direcao == DIRECAO_DIREITA ? -OPERADOR_LARGURA : OPERADOR_LARGURA;
    Rectangle fonte = {0, 0, largura, OPERADOR_ALTURA};
    int deslocamento = lado ? 7 : 8;
    DrawTextureRec(textura, fonte, (Vector2){(float)(x - deslocamento), (float)(y_pes - OPERADOR_ALTURA)}, WHITE);
}

void sprite_objeto(SpriteObjeto objeto, int x, int y) {
    if (objeto < 0 || objeto >= SPRITE_TOTAL) return;
    DrawTexture(objetos[objeto], x, y, WHITE);
}

/* ---------- botões e componentes ----------
   Cada componente desenha e devolve true no quadro em que foi clicado. */
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

static void desenhar_custo(Rectangle area, int custo, bool habilitado) {
    if (custo <= 0) return;
    const char *texto = TextFormat("%d min", custo * MINUTOS_POR_ACAO);
    float largura = render_text_width(texto, 12) + 14;
    Rectangle etiqueta = {area.x + area.width - largura - 8, area.y + (area.height - 20) / 2, largura, 20};
    DrawRectangleRounded(etiqueta, 0.5f, 4, Fade(COLOR_AMBER, habilitado ? 0.2f : 0.08f));
    render_text_centered(texto, etiqueta.x + largura / 2, etiqueta.y + 3, 12, habilitado ? COLOR_AMBER : Fade(COLOR_AMBER, 0.4f));
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
    desenhar_custo(area, custo, habilitado);
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
    DrawRectangle((int)JANELA.x + 3, 598, (int)JANELA.width - 6, 55, COLOR_BAR);
    return (Rectangle){JANELA.x + 24, JANELA.y + 70, JANELA.width - 48, 436};
}

void ui_scanlines(Rectangle area) {
    for (float y = area.y; y < area.y + area.height; y += 3) {
        DrawLineV((Vector2){area.x, y}, (Vector2){area.x + area.width, y}, Fade(BLACK, 0.12f));
    }
}
