#ifndef GAME_H
#define GAME_H

/* Uma noite em andamento na tela: personagem, cabine, painéis e o retorno visual das regras. */

#include "cabin.h"
#include "configuracoes.h"
#include "partida.h"
#include "player.h"

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
    float clock;               /* segundos reais; só anima a tela, o tempo do jogo é partida.relogio */
    bool tarefas_abertas;
    bool pediu_pausa;

    /* O que está selecionado dentro dos painéis. */
    bool receptor_em_novidades;
    int caixa_selecionada;
    int documento_selecionado;
    int mensagem_comparada;
    int programacao_selecionada;

    /* Retorno visual das ações. */
    char toast[TEXTO_AVISO];
    float toast_ate;
    float holofote_flash_ate;
    EstadoEmergencia holofote_anterior;
} GameState;

void game_init(GameState *game);
/* Prepara a tela para a noite atual da partida (posição do jogador, painéis fechados). */
void game_start_night(GameState *game);
void game_update(GameState *game, float delta);
/* Desenha a noite; os painéis também tratam os cliques aqui (interface em modo imediato). */
void game_draw(GameState *game, const Configuracoes *configuracoes);
void game_shutdown(GameState *game);
bool game_night_over(const GameState *game);

#endif
