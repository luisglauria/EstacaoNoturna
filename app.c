#include "app.h"
#include "audio.h"
#include "raylib.h"
#include "render.h"
#include "widgets.h"

void app_iniciar(App *app) {
    app->tela = TELA_MENU;
    app->voltar_para = TELA_MENU;
    app->partida_ativa = false;
    app->configuracoes = configuracoes_carregar(ARQUIVO_CONFIGURACOES);
    app->selecao = 0;
    app->pagina_tutorial = 0;
    app->tempo = 0.0f;
    ui_definir_som_clique(audio_tocar_clique);
}

/* Mantém a janela no modo pedido nas configurações (F11 também alterna). */
static void aplicar_tela_cheia(App *app) {
    if (IsKeyPressed(KEY_F11)) {
        app->configuracoes.tela_cheia = !app->configuracoes.tela_cheia;
        configuracoes_salvar(&app->configuracoes, ARQUIVO_CONFIGURACOES);
    }
    if (app->configuracoes.tela_cheia != IsWindowState(FLAG_BORDERLESS_WINDOWED_MODE)) ToggleBorderlessWindowed();
}

static void quadro_do_jogo(App *app) {
    game_update(&app->game, GetFrameTime());
    game_draw(&app->game, &app->configuracoes);
    if (game_night_over(&app->game)) {
        app->tela = TELA_RESUMO;
        audio_tocar(EFEITO_TRANSMISSAO);
    } else if (app->game.pediu_pausa) {
        app->game.pediu_pausa = false;
        app->selecao = 0;
        app->tela = TELA_PAUSA;
    }
}

void app_quadro(App *app) {
    app->tempo += GetFrameTime();
    aplicar_tela_cheia(app);
    if (app->tela != TELA_JOGO && app->tela != TELA_PAUSA) audio_definir_tensao(0.0f);
    audio_atualizar(&app->configuracoes, GetFrameTime());
    switch (app->tela) {
        case TELA_MENU: tela_menu(app); break;
        case TELA_TUTORIAL: tela_tutorial(app); break;
        case TELA_CONFIGURACOES: tela_configuracoes(app); break;
        case TELA_ABERTURA: tela_abertura(app); break;
        case TELA_JOGO: quadro_do_jogo(app); break;
        case TELA_PAUSA: tela_pausa(app); break;
        case TELA_RESUMO: tela_resumo(app); break;
        case TELA_FINAL: tela_final(app); break;
        default: break;
    }
}

void app_encerrar(App *app) {
    if (app->partida_ativa) game_shutdown(&app->game);
    configuracoes_salvar(&app->configuracoes, ARQUIVO_CONFIGURACOES);
}

bool app_deve_sair(const App *app) { return app->tela == TELA_SAIR; }
