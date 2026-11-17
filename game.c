#include "game.h"
#include "audio.h"
#include "hud.h"
#include "paineis.h"
#include "pixel.h"
#include "raylib.h"
#include "render.h"
#include "widgets.h"
#include <math.h>
#include <stdio.h>

#define DURACAO_AVISO 7.0f
#define DURACAO_CLARAO 1.8f
#define CICLO_RELAMPAGO 43.0f   /* segundos entre relâmpagos; a chuva não para */
#define ATRASO_TROVAO 1.6f      /* o som chega depois da luz, como numa tempestade de verdade */
static const Vector2 POSICAO_INICIAL = {640, 400};

void game_start_night(GameState *game) {
    player_init(&game->player, POSICAO_INICIAL);
    game->painel = PAINEL_NENHUM;
    game->nearby = EQUIPMENT_NONE;
    game->tarefas_abertas = false;
    game->pediu_pausa = false;
    game->receptor_em_novidades = true;
    game->caixa_selecionada = 0;
    game->documento_selecionado = 1;
    game->mensagem_comparada = 0;
    game->programacao_selecionada = 0;
    game->toast[0] = '\0';
    game->toast_ate = 0;
    game->holofote_flash_ate = 0;
    game->holofote_anterior = EMERGENCIA_INATIVA;
}

void game_init(GameState *game) {
    game->clock = 0.0f;
    partida_nova(&game->partida);
    game_start_night(game);
}

/* ---------- abrir e fechar painéis ---------- */

static Painel painel_do_objeto(GameState *game, Equipment equipamento) {
    switch (equipamento) {
        case EQUIPMENT_RECEIVER: return PAINEL_RECEPTOR;
        case EQUIPMENT_DESK: return PAINEL_BANCADA;
        case EQUIPMENT_TRANSMITTER: return PAINEL_TRANSMISSOR;
        case EQUIPMENT_FREQUENCY: return PAINEL_FREQUENCIA;
        case EQUIPMENT_WINDOW: return PAINEL_JANELA;
        case EQUIPMENT_DOOR: return PAINEL_PORTA;
        case EQUIPMENT_MATTRESS: return PAINEL_COLCHAO;
        case EQUIPMENT_NOTE: game->objeto_aberto = OBJETO_BILHETE; return PAINEL_OBJETO;
        case EQUIPMENT_PHOTO: game->objeto_aberto = OBJETO_FOTO; return PAINEL_OBJETO;
        case EQUIPMENT_HOME_RADIO: game->objeto_aberto = OBJETO_RADIO; return PAINEL_OBJETO;
        case EQUIPMENT_WALL_MARKS: game->objeto_aberto = OBJETO_MARCAS; return PAINEL_OBJETO;
        default: return PAINEL_NENHUM;
    }
}

static void abrir_painel(GameState *game) {
    game->painel = painel_do_objeto(game, game->nearby);
    if (game->painel == PAINEL_RECEPTOR) game->receptor_em_novidades = !game->partida.leu_novidades;
    if (game->painel != PAINEL_NENHUM) audio_tocar(EFEITO_CLIQUE);
}

static Rectangle area_do_holofote(const Partida *partida) {
    AreaChao area = partida_definicao(partida)->area_holofote;
    return (Rectangle){(float)area.x, (float)area.y, (float)area.largura, (float)area.altura};
}

/* ---------- clima e tensão ---------- */

static float fase_do_relampago(float tempo) { return fmodf(tempo + 7.0f, CICLO_RELAMPAGO); }

/* Dois clarões rápidos seguidos, como um relâmpago de verdade. */
static float intensidade_do_relampago(float tempo) {
    float fase = fase_do_relampago(tempo);
    if (fase < 0.12f) return 1.0f - fase / 0.12f;
    if (fase > 0.22f && fase < 0.32f) return 0.7f;
    return 0.0f;
}

static void tocar_trovao(const GameState *game, float delta) {
    float antes = fase_do_relampago(game->clock - delta), agora = fase_do_relampago(game->clock);
    if (antes < ATRASO_TROVAO && agora >= ATRASO_TROVAO) audio_tocar(EFEITO_TROVAO);
}

/* Quanto mais perto o perigo, mais a camada de tensão da música aparece. */
static float tensao_da_noite(const Partida *partida) {
    if (partida->fase != FASE_OPERACAO) return 0.0f;
    float tensao = 0.15f;
    int holofote = partida_acoes_ate_holofote(partida), inspecao = partida_acoes_ate_inspecao(partida);
    if (holofote >= 0) tensao = fmaxf(tensao, 1.0f - holofote * 0.15f);
    if (inspecao >= 0) tensao = fmaxf(tensao, 1.0f - inspecao * 0.12f);
    if (partida->rastreamento >= 70) tensao = fmaxf(tensao, 0.6f);
    return tensao;
}

/* ---------- atualização ---------- */

void game_update(GameState *game, float delta) {
    game->clock += delta;
    tocar_trovao(game, delta);
    audio_definir_tensao(tensao_da_noite(&game->partida));
    if (game->painel != PAINEL_NENHUM) {
        if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_E)) game->painel = PAINEL_NENHUM;
        return;
    }
    if (game->tarefas_abertas) {
        if (IsKeyPressed(KEY_TAB) || IsKeyPressed(KEY_ESCAPE)) game->tarefas_abertas = false;
        return;
    }
    if (IsKeyPressed(KEY_ESCAPE)) game->pediu_pausa = true;
    if (IsKeyPressed(KEY_TAB)) game->tarefas_abertas = true;
    player_update(&game->player, delta, cabin_is_blocked);
    if (player_step_landed(&game->player, delta)) audio_tocar(EFEITO_PASSO);
    game->nearby = cabin_near_equipment(game->player.position);
    game->partida.jogador_na_area_holofote = CheckCollisionPointRec(game->player.position, area_do_holofote(&game->partida));
    if (IsKeyPressed(KEY_E)) abrir_painel(game);
}

/* ---------- retorno das ações ---------- */

static EfeitoSonoro efeito_do_som(SomPartida som) {
    switch (som) {
        case SOM_CONFIRMA: return EFEITO_CONFIRMA;
        case SOM_ERRO: return EFEITO_ERRO;
        case SOM_TRANSMISSAO: return EFEITO_TRANSMISSAO;
        case SOM_FALHA: return EFEITO_FALHA;
        case SOM_MENSAGEM: return EFEITO_MENSAGEM;
        case SOM_PAPEL: return EFEITO_PAPEL;
        case SOM_ALERTA:
        case SOM_HOLOFOTE: return EFEITO_SIRENE;
        case SOM_INSPECAO: return EFEITO_BATIDAS;
        default: return EFEITO_TOTAL;
    }
}

/* Pega o texto e o som que a última ação deixou na partida e transforma em aviso na tela. */
static void consumir_aviso(GameState *game) {
    Partida *partida = &game->partida;
    if (partida->holofote == EMERGENCIA_RESOLVIDA && game->holofote_anterior != EMERGENCIA_RESOLVIDA)
        game->holofote_flash_ate = game->clock + DURACAO_CLARAO;
    game->holofote_anterior = partida->holofote;
    if (!partida->aviso[0]) return;
    snprintf(game->toast, sizeof(game->toast), "%s", partida->aviso);
    game->toast_ate = game->clock + DURACAO_AVISO;
    if (efeito_do_som(partida->som) != EFEITO_TOTAL) audio_tocar(efeito_do_som(partida->som));
    partida->aviso[0] = '\0';
    partida->som = SOM_NENHUM;
}

/* ---------- desenho ---------- */

static CabinScene montar_cena(const GameState *game) {
    const Partida *partida = &game->partida;
    float clarao = (game->holofote_flash_ate - game->clock) / DURACAO_CLARAO;
    return (CabinScene){
        .curtain_closed = partida->cortina_fechada,
        .equipment_on = partida->equipamentos_ligados,
        .evidence_hidden = partida->provas_escondidas,
        .beam_warning = partida->holofote == EMERGENCIA_AVISADA,
        .beam_flash = clarao > 0 ? clarao : 0,
        .beam_area = area_do_holofote(partida),
        .agents_at_door = partida->inspecao == EMERGENCIA_AVISADA,
        .lightning = intensidade_do_relampago(game->clock),
    };
}

static const char *titulo_do_painel(const GameState *game) {
    switch (game->painel) {
        case PAINEL_RECEPTOR: return "RECEPTOR";
        case PAINEL_BANCADA: return "BANCADA";
        case PAINEL_TRANSMISSOR: return "TRANSMISSOR · ROTEIRO DA NOITE";
        case PAINEL_FREQUENCIA: return "PAINEL DE FREQUÊNCIA";
        case PAINEL_JANELA: return "JANELA";
        case PAINEL_PORTA: return "PORTA";
        case PAINEL_COLCHAO: return "COLCHÃO";
        default: return campanha_objeto(game->objeto_aberto)->titulo;
    }
}

static void desenhar_painel(GameState *game) {
    if (game->painel == PAINEL_NENHUM) return;
    bool fechar = false;
    Rectangle area = ui_janela(titulo_do_painel(game), &fechar);
    switch (game->painel) {
        case PAINEL_RECEPTOR: painel_receptor(game, area); break;
        case PAINEL_BANCADA: painel_bancada(game, area); break;
        case PAINEL_TRANSMISSOR: painel_transmissor(game, area); break;
        case PAINEL_FREQUENCIA: painel_frequencia(game, area); break;
        case PAINEL_JANELA: painel_janela(game, area); break;
        case PAINEL_PORTA: painel_porta(game, area); break;
        case PAINEL_COLCHAO: painel_colchao(game, area); break;
        default: painel_objeto(game, area); break;
    }
    if (fechar) game->painel = PAINEL_NENHUM;
}

static void desenhar_jogador(const void *jogador) { player_draw((const Player *)jogador); }

static void desenhar_mundo(const GameState *game) {
    CabinScene cena = montar_cena(game);
    Equipment destacado = game->painel == PAINEL_NENHUM ? game->nearby : EQUIPMENT_NONE;
    float relampago = cena.lightning;
    Color ambiente = ColorLerp((Color){52, 54, 84, 255}, (Color){120, 128, 170, 255}, cena.curtain_closed ? 0 : relampago);
    pixel_luz_comecar(ambiente);
    cabin_draw_lights(game->clock, &cena);
    /* Halo fraco em volta do operador: no escuro total ele sumiria da tela. */
    pixel_luz((float)px(game->player.position.x), (float)(player_feet_pixel(&game->player) - 10), 34, (Color){255, 214, 170, 255}, 0.4f);
    pixel_luz_terminar();
    pixel_mundo_comecar(BLACK);
    cabin_draw(game->clock, &cena, player_feet_pixel(&game->player), desenhar_jogador, &game->player);
    pixel_aplicar_luz();
    cabin_draw_emissive(game->clock, destacado, &cena);
    pixel_vinheta(1.0f);
    pixel_mundo_terminar();
}

/* Rótulo da área do holofote, em texto nítido por cima do mundo em pixel art. */
static void desenhar_rotulo_do_holofote(const GameState *game) {
    if (game->partida.holofote != EMERGENCIA_AVISADA || game->partida.cortina_fechada) return;
    Rectangle area = area_do_holofote(&game->partida);
    render_text("A LUZ VAI PASSAR AQUI", area.x + 11, area.y + area.height - 25, 14, Fade(BLACK, 0.8f));
    render_text("A LUZ VAI PASSAR AQUI", area.x + 10, area.y + area.height - 26, 14, COLOR_AMBER);
}

void game_draw(GameState *game, const Configuracoes *configuracoes) {
    desenhar_mundo(game);
    desenhar_rotulo_do_holofote(game);
    bool livre = game->painel == PAINEL_NENHUM && !game->tarefas_abertas;
    if (livre && game->nearby != EQUIPMENT_NONE) hud_desenhar_balao(game->player.position, cabin_equipment_label(game->nearby), game->clock);
    hud_desenhar_topo(&game->partida);
    hud_desenhar_rodape(&game->partida, game->painel != PAINEL_NENHUM);
    hud_desenhar_alerta(&game->partida, game->clock);
    if (livre && configuracoes->dicas) hud_desenhar_dica(&game->partida);
    desenhar_painel(game);
    consumir_aviso(game);
    float restante = game->toast_ate - game->clock;
    if (!game->tarefas_abertas) hud_desenhar_aviso(game->toast, restante > 1.0f ? 1.0f : restante, game->painel != PAINEL_NENHUM);
    else hud_desenhar_tarefas(&game->partida);
}

void game_shutdown(GameState *game) { partida_liberar(&game->partida); }

bool game_night_over(const GameState *game) { return game->partida.fase == FASE_ENCERRADA; }
