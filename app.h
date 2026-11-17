#ifndef APP_H
#define APP_H

/* Máquina de telas do jogo: menu, tutorial, configurações, noite, pausa, resumo e final. */

#include "configuracoes.h"
#include "game.h"

typedef enum {
    TELA_MENU,
    TELA_TUTORIAL,
    TELA_CONFIGURACOES,
    TELA_ABERTURA,
    TELA_JOGO,
    TELA_PAUSA,
    TELA_RESUMO,
    TELA_FINAL,
    TELA_SAIR
} Tela;

typedef struct {
    Tela tela;
    Tela voltar_para;   /* para onde o tutorial e as configurações voltam (menu ou pausa) */
    GameState game;
    bool partida_ativa;
    Configuracoes configuracoes;
    int selecao;        /* item destacado nos menus, para navegar pelo teclado */
    int pagina_tutorial;
    float tempo;
} App;

void app_iniciar(App *app);
/* Atualiza e desenha um quadro da tela atual. */
void app_quadro(App *app);
void app_encerrar(App *app);
bool app_deve_sair(const App *app);

/* Telas (telas.c, tutorial.c, resumo.c). Cada uma desenha e trata a própria entrada. */
void tela_menu(App *app);
void tela_configuracoes(App *app);
void tela_abertura(App *app);
void tela_pausa(App *app);
void tela_final(App *app);
void tela_tutorial(App *app);
void tela_resumo(App *app);
/* Fundo noturno animado compartilhado pelas telas de menu. */
void telas_desenhar_fundo(float tempo);
/* Lista vertical de botões navegável por setas/ENTER e mouse; devolve o índice escolhido ou -1. */
int telas_menu_vertical(App *app, const char *const opcoes[], int total, float x, float y, float largura);

#endif
