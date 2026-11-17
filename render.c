#include "render.h"
#include "rlgl.h"
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

/* Fontes tentadas em ordem; a fonte padrão da raylib fica como último recurso. */
static const char *FONT_PATHS[] = {
    "assets/font.ttf",
    "C:/Windows/Fonts/consola.ttf",
    "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",
};
#define FONT_BASE_SIZE 48
#define FONT_FIRST_CODEPOINT 32
#define FONT_CODEPOINT_COUNT 224 /* ASCII + Latin-1: cobre ç, ã, ê e os demais acentos */

static Font game_font;
static bool has_custom_font = false;
static RenderTexture2D frame_target;

static bool try_load_font(const char *path) {
    if (!FileExists(path)) return false;
    int codepoints[FONT_CODEPOINT_COUNT];
    for (int i = 0; i < FONT_CODEPOINT_COUNT; i++) codepoints[i] = FONT_FIRST_CODEPOINT + i;
    Font font = LoadFontEx(path, FONT_BASE_SIZE, codepoints, FONT_CODEPOINT_COUNT);
    if (font.texture.id == GetFontDefault().texture.id) return false; /* falha devolve a padrão */
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

/* Margem que centraliza a imagem quando a janela tem outra proporção (faixas pretas). */
static Vector2 frame_offset(float scale) {
    return (Vector2){(GetScreenWidth() - VIRTUAL_WIDTH * scale) * 0.5f,
                     (GetScreenHeight() - VIRTUAL_HEIGHT * scale) * 0.5f};
}

void render_begin_frame(void) { BeginTextureMode(frame_target); }

void render_pause_frame(void) { EndTextureMode(); }

void render_resume_frame(void) { BeginTextureMode(frame_target); }

void render_copy_texture(Texture2D texture, Rectangle dest) {
    /* Altura negativa porque a textura de render fica de cabeça para baixo no OpenGL.
       Luz e vinheta deixam o alfa abaixo de 1; copiado com mistura normal, o que estivesse
       atrás vazaria. Aqui o RGB é copiado e o alfa do destino fica como está (opaco). */
    Rectangle source = {0, 0, (float)texture.width, (float)-texture.height};
    rlSetBlendFactorsSeparate(RL_ONE, RL_ZERO, RL_ZERO, RL_ONE, RL_FUNC_ADD, RL_FUNC_ADD);
    BeginBlendMode(BLEND_CUSTOM_SEPARATE);
    DrawTexturePro(texture, source, dest, (Vector2){0, 0}, 0.0f, WHITE);
    EndBlendMode();
}

void render_end_frame(void) {
    EndTextureMode();
    float scale = frame_scale();
    Vector2 offset = frame_offset(scale);
    BeginDrawing();
    ClearBackground(BLACK);
    render_copy_texture(frame_target.texture, (Rectangle){offset.x, offset.y, VIRTUAL_WIDTH * scale, VIRTUAL_HEIGHT * scale});
    EndDrawing();
}

Vector2 render_mouse_position(void) {
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

/* Copia a próxima palavra (até espaço ou quebra de linha) e devolve onde ela terminou. */
static const char *next_word(const char *cursor, char *word, size_t word_size) {
    const char *start = cursor;
    while (*cursor && *cursor != ' ' && *cursor != '\n') cursor++;
    size_t length = (size_t)(cursor - start);
    if (length >= word_size) length = word_size - 1;
    memcpy(word, start, length);
    word[length] = '\0';
    return cursor;
}

/* Copia até caber no destino (linhas maiores que o buffer são cortadas, não estouram). */
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

/* Percorre o texto palavra por palavra; desenha só se draw for true. Devolve a altura total. */
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
