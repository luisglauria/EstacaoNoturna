/* Cabine: geometria (em pixels do mundo), camada fixa, luzes, ordem de profundidade e janela/porta. */
#include "cabin_internal.h"
#include "render.h"
#include "sprites.h"
#include <math.h>
#include <stddef.h>

#define INTERACT_REACH 46 /* distância (tela virtual) até a borda do objeto; folgada para não exigir encostar */
#define PAREDE_TOPO 14    /* onde começa a face da parede de cima */

static const PxRect PISO = {49, 34, 328, 182};
static const PxRect VIDRO = {188, 21, 46, 10};
static RenderTexture2D camada_fixa;

static const EquipmentSpot SPOTS[] = {
    {EQUIPMENT_RECEIVER, {58, 36, 74, 26}, "RECEPTOR", true},
    {EQUIPMENT_TRANSMITTER, {294, 36, 74, 28}, "TRANSMISSOR", true},
    {EQUIPMENT_DESK, {156, 182, 114, 30}, "BANCADA", true},
    {EQUIPMENT_FREQUENCY, {356, 132, 20, 66}, "PAINEL DE FREQUÊNCIA", true},
    {EQUIPMENT_WINDOW, {186, 20, 50, 14}, "JANELA", false},
    {EQUIPMENT_DOOR, {377, 84, 8, 30}, "PORTA", false},
    {EQUIPMENT_MATTRESS, {55, 152, 40, 58}, "COLCHÃO", true},
    {EQUIPMENT_NOTE, {51, 96, 18, 34}, "ARQUIVO DE AÇO", true},
    {EQUIPMENT_PHOTO, {142, 22, 8, 8}, "FOTOGRAFIA", false},
    {EQUIPMENT_HOME_RADIO, {100, 202, 12, 8}, "RÁDIO DOMÉSTICO", true},
    {EQUIPMENT_WALL_MARKS, {262, 22, 12, 9}, "MARCAS NA PAREDE", false},
};
#define SPOT_COUNT ((int)(sizeof(SPOTS) / sizeof(SPOTS[0])))

static Rectangle virtual_rect(PxRect r) { return (Rectangle){px_virtual(r.x), px_virtual(r.y), px_virtual(r.w), px_virtual(r.h)}; }

/* ---------- consultas ---------- */

const EquipmentSpot *cabin_spots(int *count) {
    *count = SPOT_COUNT;
    return SPOTS;
}

const EquipmentSpot *cabin_spot(Equipment id) {
    for (int i = 0; i < SPOT_COUNT; i++) {
        if (SPOTS[i].id == id) return &SPOTS[i];
    }
    return NULL;
}

Rectangle cabin_floor(void) { return virtual_rect(PISO); }

bool cabin_is_blocked(Rectangle hitbox) {
    Rectangle piso = cabin_floor();
    bool dentro = hitbox.x >= piso.x && hitbox.y >= piso.y && hitbox.x + hitbox.width <= piso.x + piso.width &&
                  hitbox.y + hitbox.height <= piso.y + piso.height;
    if (!dentro) return true;
    for (int i = 0; i < SPOT_COUNT; i++) {
        if (SPOTS[i].solid && CheckCollisionRecs(hitbox, virtual_rect(SPOTS[i].body))) return true;
    }
    return false;
}

static float distance_to_rect(Vector2 point, Rectangle box) {
    float dx = fmaxf(fmaxf(box.x - point.x, 0), point.x - (box.x + box.width));
    float dy = fmaxf(fmaxf(box.y - point.y, 0), point.y - (box.y + box.height));
    return sqrtf(dx * dx + dy * dy);
}

Equipment cabin_near_equipment(Vector2 player_position) {
    Equipment nearest = EQUIPMENT_NONE;
    float best = INTERACT_REACH;
    for (int i = 0; i < SPOT_COUNT; i++) {
        float distance = distance_to_rect(player_position, virtual_rect(SPOTS[i].body));
        if (distance > best) continue;
        best = distance;
        nearest = SPOTS[i].id;
    }
    return nearest;
}

const char *cabin_equipment_label(Equipment equipment) {
    const EquipmentSpot *spot = cabin_spot(equipment);
    return spot ? spot->label : "";
}

/* ---------- camada fixa: piso, tapete e paredes (desenhada uma vez) ---------- */

static void desenhar_tabuas(void) {
    for (int y = PISO.y, linha = 0; y < PISO.y + PISO.h; y += 7, linha++) {
        Color tom = linha % 2 ? (Color){92, 66, 48, 255} : (Color){84, 60, 44, 255};
        px_ret(PISO.x, y, PISO.w, 7, tom);
        px_hlinha(PISO.x, y, PISO.w, (Color){40, 28, 22, 255});
        for (int x = PISO.x + (int)(px_ruido(linha, 3) % 60); x < PISO.x + PISO.w; x += 60) px_vlinha(x, y, 7, (Color){40, 28, 22, 255});
        for (int veio = 0; veio < 30; veio++) {
            unsigned ruido = px_ruido(veio, linha);
            px_hlinha(PISO.x + (int)(ruido % (unsigned)PISO.w), y + 2 + (int)((ruido >> 8) % 4), 2 + (int)((ruido >> 12) % 5), px_escurecer(tom, 0.82f));
        }
    }
}

static void desenhar_tijolos(int x, int y, int largura, int altura) {
    px_ret(x, y, largura, altura, (Color){86, 64, 66, 255});
    for (int linha = 0; linha * 4 < altura; linha++) {
        px_hlinha(x, y + linha * 4, largura, (Color){46, 34, 38, 255});
        for (int coluna = (linha % 2) * 5; coluna < largura; coluna += 10) px_vlinha(x + coluna, y + linha * 4, 4, (Color){46, 34, 38, 255});
    }
    for (int mancha = 0; mancha < 60; mancha++) {
        unsigned ruido = px_ruido(mancha, 77);
        px_ponto(x + (int)(ruido % (unsigned)largura), y + (int)((ruido >> 9) % (unsigned)altura), (Color){60, 44, 48, 255});
    }
}

static void desenhar_paredes(void) {
    const Color topo = {30, 26, 34, 255}, quina = {58, 50, 60, 255};
    desenhar_tijolos(PISO.x - 8, PAREDE_TOPO, PISO.w + 16, PISO.y - PAREDE_TOPO);
    px_ret(PISO.x, PISO.y - 2, PISO.w, 2, (Color){54, 38, 30, 255}); /* rodapé */
    px_ret(PISO.x - 8, PAREDE_TOPO - 3, PISO.w + 16, 3, topo);
    px_ret(PISO.x - 8, PISO.y, 8, PISO.h + 6, topo);
    px_ret(PISO.x + PISO.w, PISO.y, 8, PISO.h + 6, topo);
    px_ret(PISO.x - 8, PISO.y + PISO.h, PISO.w + 16, 6, topo);
    px_vlinha(PISO.x - 1, PISO.y, PISO.h, quina);
    px_vlinha(PISO.x + PISO.w, PISO.y, PISO.h, quina);
    px_hlinha(PISO.x, PISO.y + PISO.h, PISO.w, quina);
}

/* Fotografia e marcas ficam na face da parede e não mudam: entram na camada fixa. */
static void desenhar_detalhes_de_parede(void) {
    const EquipmentSpot *marcas = cabin_spot(EQUIPMENT_WALL_MARKS);
    for (int risco = 0; risco < 7; risco++) {
        px_vlinha(marcas->body.x + (risco % 4) * 2 + (risco / 4) * 9, marcas->body.y + 1, 6, (Color){196, 182, 160, 255});
    }
    px_hlinha(marcas->body.x - 1, marcas->body.y + 3, 8, (Color){196, 182, 160, 255});
    const EquipmentSpot *foto = cabin_spot(EQUIPMENT_PHOTO);
    sprite_objeto(SPRITE_FOTO, foto->body.x, foto->body.y);
}

static void ordenar_por_profundidade(void);

void cabin_iniciar(void) {
    ordenar_por_profundidade();
    camada_fixa = LoadRenderTexture(MUNDO_LARGURA, MUNDO_ALTURA);
    BeginTextureMode(camada_fixa);
    ClearBackground((Color){6, 6, 10, 255});
    desenhar_tabuas();
    cabin_draw_rug();
    desenhar_paredes();
    desenhar_detalhes_de_parede();
    EndTextureMode();
}

void cabin_encerrar(void) { UnloadRenderTexture(camada_fixa); }

/* ---------- janela, porta e holofote ---------- */

static void desenhar_cortina(bool fechada, Color pano, Color dobra) {
    if (fechada) {
        px_ret(VIDRO.x - 3, VIDRO.y - 2, VIDRO.w + 6, VIDRO.h + 5, pano);
        for (int x = VIDRO.x - 1; x < VIDRO.x + VIDRO.w + 3; x += 4) px_vlinha(x, VIDRO.y - 2, VIDRO.h + 5, dobra);
        return;
    }
    px_ret(VIDRO.x - 9, VIDRO.y - 2, 7, VIDRO.h + 7, pano);
    for (int x = VIDRO.x - 8; x < VIDRO.x - 2; x += 2) px_vlinha(x, VIDRO.y - 2, VIDRO.h + 7, dobra);
}

static void desenhar_janela(const CabinScene *scene) {
    px_ret(VIDRO.x - 2, VIDRO.y - 2, VIDRO.w + 4, VIDRO.h + 4, (Color){70, 56, 50, 255});
    px_ret(VIDRO.x, VIDRO.y, VIDRO.w, VIDRO.h, (Color){20, 26, 44, 255});
    px_ret(VIDRO.x - 3, VIDRO.y + VIDRO.h + 2, VIDRO.w + 6, 2, (Color){96, 78, 66, 255}); /* peitoril */
    desenhar_cortina(scene->curtain_closed, (Color){120, 38, 44, 255}, (Color){84, 24, 30, 255});
}

/* Céu e chuva brilham sozinhos: são desenhados depois da luz. */
static void desenhar_ceu(float time, const CabinScene *scene) {
    if (scene->curtain_closed) return;
    Color ceu = ColorLerp((Color){18, 24, 46, 255}, (Color){214, 222, 255, 255}, fmaxf(scene->lightning, scene->beam_flash));
    px_ret(VIDRO.x, VIDRO.y, VIDRO.w, VIDRO.h, ceu);
    for (int gota = 0; gota < 14; gota++) {
        unsigned ruido = px_ruido(gota, 11);
        int x = VIDRO.x + (int)(ruido % (unsigned)VIDRO.w);
        int y = VIDRO.y + (int)fmodf(time * (38.0f + (float)(ruido % 20)) + (float)(ruido >> 8), (float)VIDRO.h);
        px_vlinha(x, y, 2, (Color){120, 140, 190, 255});
    }
    px_vlinha(VIDRO.x + VIDRO.w / 2, VIDRO.y, VIDRO.h, (Color){70, 56, 50, 255});
    px_hlinha(VIDRO.x, VIDRO.y + VIDRO.h / 2, VIDRO.w, (Color){70, 56, 50, 255});
    desenhar_cortina(false, (Color){60, 20, 26, 255}, (Color){40, 14, 18, 255});
}

static void desenhar_porta(void) {
    const EquipmentSpot *porta = cabin_spot(EQUIPMENT_DOOR);
    PxRect r = porta->body;
    px_ret(r.x, r.y, r.w, r.h, (Color){82, 54, 38, 255});
    px_borda(r.x, r.y, r.w, r.h, COR_CONTORNO);
    px_vlinha(r.x + 4, r.y + 2, r.h - 4, (Color){62, 40, 28, 255});
    px_ponto(r.x + 1, r.y + r.h / 2 + 2, (Color){214, 180, 100, 255});
}

/* Fresta de luz do corredor; com agentes, fica vermelha e aparecem sombras de pés. */
static void desenhar_fresta(float time, const CabinScene *scene) {
    PxRect r = cabin_spot(EQUIPMENT_DOOR)->body;
    Color luz = scene->agents_at_door ? COR_VERMELHA : (Color){200, 150, 90, 255};
    px_vlinha(r.x - 1, r.y + 1, r.h - 2, luz);
    if (!scene->agents_at_door) return;
    int passo = (int)(time * 2.0f) % 2;
    px_ret(r.x - 1, r.y + 6 + passo, 1, 4, (Color){10, 6, 8, 255});
    px_ret(r.x - 1, r.y + 18 - passo, 1, 4, (Color){10, 6, 8, 255});
}

static void tracejado(PxRect area, Color cor, int fase) {
    for (int x = area.x + fase; x < area.x + area.w; x += 6) {
        px_hlinha(x, area.y, 3, cor);
        px_hlinha(x, area.y + area.h - 1, 3, cor);
    }
    for (int y = area.y + fase; y < area.y + area.h; y += 6) {
        px_vlinha(area.x, y, 3, cor);
        px_vlinha(area.x + area.w - 1, y, 3, cor);
    }
}

static PxRect area_em_pixels(Rectangle area) { return (PxRect){px(area.x), px(area.y), px(area.width), px(area.height)}; }

static void desenhar_area_do_holofote(float time, const CabinScene *scene) {
    if (!scene->beam_warning || scene->curtain_closed) return;
    tracejado(area_em_pixels(scene->beam_area), Fade(COR_AMBAR, 0.55f + 0.35f * sinf(time * 4.0f)), (int)(time * 6.0f) % 6);
}

/* ---------- luzes ---------- */

static Vector2 centro_px(const EquipmentSpot *spot, int dx, int dy) {
    return (Vector2){(float)(spot->body.x + dx), (float)(spot->body.y + dy)};
}

static void luzes_dos_equipamentos(float time, const CabinScene *scene) {
    if (!scene->equipment_on) return;
    Vector2 receptor = centro_px(cabin_spot(EQUIPMENT_RECEIVER), 18, 10);
    Vector2 transmissor = centro_px(cabin_spot(EQUIPMENT_TRANSMITTER), 40, 8);
    Vector2 painel = centro_px(cabin_spot(EQUIPMENT_FREQUENCY), 10, 14);
    pixel_luz(receptor.x, receptor.y, 40, (Color){90, 230, 160, 255}, 0.55f);
    pixel_luz(transmissor.x, transmissor.y, 30, (Color){255, 70, 60, 255}, 0.4f + 0.2f * sinf(time * 4.0f));
    pixel_luz(transmissor.x - 18, transmissor.y + 2, 32, (Color){255, 170, 80, 255}, 0.3f);
    pixel_luz(painel.x, painel.y, 26, (Color){90, 230, 160, 255}, 0.4f);
}

static void luz_da_janela(float time, const CabinScene *scene) {
    if (scene->curtain_closed) return;
    Vector2 janela = {(float)(VIDRO.x + VIDRO.w / 2), (float)PISO.y};
    float relampago = scene->lightning;
    pixel_feixe(janela, (Vector2){janela.x + 18, janela.y + 110}, 96, (Color){110, 130, 210, 255}, 0.4f + 0.6f * relampago);
    if (relampago > 0) pixel_luz(janela.x, janela.y + 40, 200, (Color){170, 190, 255, 255}, relampago * 0.7f);
    PxRect area = area_em_pixels(scene->beam_area);
    Vector2 alvo = {(float)(area.x + area.w / 2), (float)(area.y + area.h / 2)};
    if (scene->beam_warning) {
        /* O feixe já varre a fachada: entra fraco e oscilando, anunciando a passagem. */
        float varredura = sinf(time * 1.3f) * area.w * 0.6f;
        pixel_feixe(janela, (Vector2){alvo.x + varredura, alvo.y}, 60, (Color){255, 246, 200, 255}, 0.18f);
    }
    if (scene->beam_flash > 0) pixel_feixe(janela, alvo, (float)area.w * 1.3f, (Color){255, 250, 220, 255}, scene->beam_flash);
}

void cabin_draw_lights(float time, const CabinScene *scene) {
    const EquipmentSpot *bancada = cabin_spot(EQUIPMENT_DESK);
    float tremor = 0.92f + 0.08f * sinf(time * 17.0f) * sinf(time * 5.3f);
    pixel_luz(bancada->body.x + 100, bancada->body.y + 4, 88, (Color){255, 190, 120, 255}, 0.95f * tremor);
    pixel_luz(213, 110, 130, (Color){255, 200, 140, 255}, 0.22f * tremor); /* lâmpada do teto, fraca */
    luzes_dos_equipamentos(time, scene);
    luz_da_janela(time, scene);
    PxRect porta = cabin_spot(EQUIPMENT_DOOR)->body;
    Color corredor = scene->agents_at_door ? (Color){255, 60, 50, 255} : (Color){255, 190, 120, 255};
    pixel_luz(porta.x - 2, porta.y + porta.h / 2, 26, corredor, scene->agents_at_door ? 0.5f + 0.2f * sinf(time * 6.0f) : 0.3f);
}

/* ---------- cena com profundidade ---------- */

static int base_do_spot(const EquipmentSpot *spot) { return spot->body.y + spot->body.h; }

/* Índices dos objetos do fundo para a frente (pela linha da base), calculados uma vez. */
static int ordem_de_desenho[SPOT_COUNT];

static void ordenar_por_profundidade(void) {
    for (int i = 0; i < SPOT_COUNT; i++) {
        int j = i;
        while (j > 0 && base_do_spot(&SPOTS[ordem_de_desenho[j - 1]]) > base_do_spot(&SPOTS[i])) {
            ordem_de_desenho[j] = ordem_de_desenho[j - 1];
            j--;
        }
        ordem_de_desenho[j] = i;
    }
}

void cabin_draw(float time, const CabinScene *scene, int player_feet, void (*draw_player)(const void *), const void *player) {
    DrawTextureRec(camada_fixa.texture, (Rectangle){0, 0, MUNDO_LARGURA, -MUNDO_ALTURA}, (Vector2){0, 0}, WHITE);
    desenhar_janela(scene);
    desenhar_porta();
    desenhar_area_do_holofote(time, scene);
    cabin_draw_chair();
    bool jogador_desenhado = false;
    for (int i = 0; i < SPOT_COUNT; i++) {
        const EquipmentSpot *spot = &SPOTS[ordem_de_desenho[i]];
        if (!jogador_desenhado && spot->solid && base_do_spot(spot) > player_feet) {
            draw_player(player);
            jogador_desenhado = true;
        }
        cabin_draw_spot(spot, time, scene);
    }
    if (!jogador_desenhado) draw_player(player);
}

/* Partículas de poeira flutuando na luz do abajur. */
static void desenhar_poeira(float time) {
    const EquipmentSpot *bancada = cabin_spot(EQUIPMENT_DESK);
    for (int i = 0; i < 12; i++) {
        unsigned ruido = px_ruido(i, 5);
        float x = bancada->body.x + 70 + (float)(ruido % 60) + sinf(time * 0.4f + i) * 6;
        float y = bancada->body.y - 30 + fmodf((float)((ruido >> 8) % 50) - time * 3.0f + 50, 50);
        px_ponto((int)x, (int)y, Fade((Color){255, 220, 170, 255}, 0.35f));
    }
}

static void desenhar_destaque(Equipment highlighted, float time) {
    const EquipmentSpot *spot = cabin_spot(highlighted);
    if (!spot) return;
    PxRect r = spot->body;
    px_borda(r.x - 2, r.y - 2, r.w + 4, r.h + 4, Fade(COR_AMBAR, 0.45f + 0.35f * sinf(time * 5.0f)));
}

void cabin_draw_emissive(float time, Equipment highlighted, const CabinScene *scene) {
    desenhar_ceu(time, scene);
    desenhar_fresta(time, scene);
    for (int i = 0; i < SPOT_COUNT; i++) cabin_draw_spot_glow(&SPOTS[i], time, scene);
    desenhar_poeira(time);
    desenhar_area_do_holofote(time, scene);
    desenhar_destaque(highlighted, time);
}
