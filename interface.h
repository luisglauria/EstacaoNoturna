#ifndef INTERFACE_H
#define INTERFACE_H

#include "partida.h"
#include "raylib.h"
#include <stdbool.h>

/* Declarações da parte gráfica (raylib). O jogo desenha sempre em 1280x720
   e a imagem é escalada para o tamanho da janela. */
#define VIRTUAL_WIDTH 1280
#define VIRTUAL_HEIGHT 720

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

#define PIXEL_ESCALA 3
#define MUNDO_LARGURA 427
#define MUNDO_ALTURA 240

typedef struct {
    int volume_geral;
    int volume_musica;
    int volume_efeitos;
    bool tela_cheia;
    bool dicas;
} Configuracoes;

typedef enum {
    EFEITO_CLIQUE,
    EFEITO_CONFIRMA,
    EFEITO_ERRO,
    EFEITO_TRANSMISSAO,
    EFEITO_FALHA,
    EFEITO_MENSAGEM,
    EFEITO_SIRENE,
    EFEITO_BATIDAS,
    EFEITO_PAPEL,
    EFEITO_PASSO,
    EFEITO_TROVAO,
    EFEITO_TOTAL
} EfeitoSonoro;

typedef enum { DIRECAO_BAIXO, DIRECAO_CIMA, DIRECAO_ESQUERDA, DIRECAO_DIREITA } Direcao;
typedef enum { SPRITE_FOTO, SPRITE_RADIO_DOMESTICO, SPRITE_TOTAL } SpriteObjeto;

typedef struct {
    Vector2 position;
    Vector2 facing;
    float speed;
    float walk_time;
    bool moving;
} Player;

typedef enum {
    EQUIPMENT_NONE,
    EQUIPMENT_RECEIVER,
    EQUIPMENT_DESK,
    EQUIPMENT_TRANSMITTER,
    EQUIPMENT_FREQUENCY,
    EQUIPMENT_WINDOW,
    EQUIPMENT_DOOR,
    EQUIPMENT_MATTRESS,
    EQUIPMENT_NOTE,
    EQUIPMENT_PHOTO,
    EQUIPMENT_HOME_RADIO,
    EQUIPMENT_WALL_MARKS,
    EQUIPMENT_COUNT
} Equipment;

typedef struct {
    bool curtain_closed;
    bool equipment_on;
    bool evidence_hidden;
    bool beam_warning;
    float beam_flash;
    Rectangle beam_area;
    bool agents_at_door;
    float lightning;
} CabinScene;

typedef enum {
    PAINEL_NENHUM,
    PAINEL_RECEPTOR,
    PAINEL_BANCADA,
    PAINEL_TRANSMISSOR,
    PAINEL_FREQUENCIA,
    PAINEL_JANELA,
    PAINEL_PORTA,
    PAINEL_COLCHAO,
    PAINEL_OBJETO
} Painel;

typedef struct {
    Player player;
    Partida partida;
    Painel painel;
    Equipment nearby;
    ObjetoNarrativo objeto_aberto;
    float clock;
    bool tarefas_abertas;
    bool pediu_pausa;

    bool receptor_em_novidades;
    int caixa_selecionada;
    int documento_selecionado;
    int mensagem_comparada;
    int programacao_selecionada;

    char toast[TEXTO_AVISO];
    float toast_ate;
    float holofote_flash_ate;
    EstadoEmergencia holofote_anterior;
} GameState;

/* graficos.c: tela virtual, texto, pixel art, sprites e botões */
void render_init(void);
void render_shutdown(void);
void render_begin_frame(void);
void render_end_frame(void);
void render_text(const char *text, float x, float y, float size, Color color);
void render_text_centered(const char *text, float center_x, float y, float size, Color color);
float render_text_width(const char *text, float size);
float render_text_wrapped(const char *text, Rectangle area, float size, Color color);
float render_text_wrapped_height(const char *text, float width, float size);

int px(float coordenada_virtual);
float px_virtual(int pixel);
void pixel_iniciar(void);
void pixel_encerrar(void);
void pixel_luz_comecar(Color ambiente);
void pixel_luz(float x, float y, float raio, Color cor, float forca);
void pixel_feixe(Vector2 origem, Vector2 alvo, float largura, Color cor, float forca);
void pixel_luz_terminar(void);
void pixel_mundo_comecar(Color fundo);
void pixel_aplicar_luz(void);
void pixel_vinheta(float forca);
void pixel_mundo_terminar(void);
void px_ret(int x, int y, int largura, int altura, Color cor);
void px_borda(int x, int y, int largura, int altura, Color cor);
void px_hlinha(int x, int y, int largura, Color cor);
void px_vlinha(int x, int y, int altura, Color cor);
void px_ponto(int x, int y, Color cor);
void px_movel(int x, int y, int largura, int altura, int altura_frente, Color tampo, Color frente);
unsigned px_ruido(int x, int y);
Color px_escurecer(Color cor, float fator);

void sprites_iniciar(void);
void sprites_encerrar(void);
void sprite_operador(int x, int y_pes, Direcao direcao, int passo);
void sprite_objeto(SpriteObjeto objeto, int x, int y);

void ui_definir_som_clique(void (*tocar)(void));
bool ui_mouse_sobre(Rectangle area);
bool ui_clicou(Rectangle area);
bool ui_botao(Rectangle area, const char *rotulo, bool habilitado);
bool ui_botao_custo(Rectangle area, const char *rotulo, int custo, bool habilitado);
bool ui_botao_principal(Rectangle area, const char *rotulo, int custo, bool habilitado);
bool ui_item_lista(Rectangle area, const char *titulo, const char *detalhe, bool selecionado, Color cor_detalhe);
void ui_etiqueta(float x, float y, const char *texto, Color cor);
void ui_medidor(Rectangle area, const char *rotulo, int valor, Color cor);
bool ui_alternador(Rectangle area, const char *rotulo, bool *valor);
bool ui_deslizador(Rectangle area, const char *rotulo, int *valor);
float ui_tecla(const char *tecla, float x, float y);
float ui_dica_tecla(const char *tecla, const char *acao, float x, float y);
Rectangle ui_janela(const char *titulo, bool *fechar);
void ui_scanlines(Rectangle area);

/* audio.c: música e efeitos gerados por código */
void audio_iniciar(void);
void audio_encerrar(void);
void audio_atualizar(const Configuracoes *configuracoes, float delta);
void audio_definir_tensao(float tensao);
void audio_tocar(EfeitoSonoro efeito);
void audio_tocar_clique(void);

/* cabine.c: cabine, colisão e personagem */
void cabin_iniciar(void);
void cabin_encerrar(void);
bool cabin_is_blocked(Rectangle hitbox);
Equipment cabin_near_equipment(Vector2 player_position);
const char *cabin_equipment_label(Equipment equipment);
void cabin_draw_lights(float time, const CabinScene *scene);
void cabin_draw(float time, const CabinScene *scene, int player_feet, void (*draw_player)(const void *), const void *player);
void cabin_draw_emissive(float time, Equipment highlighted, const CabinScene *scene);

void player_init(Player *player, Vector2 position);
void player_update(Player *player, float delta, bool (*is_blocked)(Rectangle hitbox));
int player_feet_pixel(const Player *player);
void player_draw(const Player *player);
bool player_step_landed(const Player *player, float delta);

/* noite.c: a noite na tela (HUD e painéis) */
void game_init(GameState *game);
void game_start_night(GameState *game);
void game_update(GameState *game, float delta);
void game_draw(GameState *game, const Configuracoes *configuracoes);
void game_shutdown(GameState *game);
bool game_night_over(const GameState *game);

#endif
