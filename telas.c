/* Menu inicial, configurações, abertura da noite, pausa e final. */
#include "app.h"
#include "audio.h"
#include "raylib.h"
#include "render.h"
#include "widgets.h"
#include <math.h>

/* ---------- peças compartilhadas ---------- */

int telas_menu_vertical(App *app, const char *const opcoes[], int total, float x, float y, float largura) {
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) app->selecao = (app->selecao + 1) % total;
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) app->selecao = (app->selecao + total - 1) % total;
    if (app->selecao >= total) app->selecao = 0;
    int escolhida = -1;
    for (int i = 0; i < total; i++) {
        Rectangle botao = {x, y + i * 62, largura, 50};
        if (ui_mouse_sobre(botao)) app->selecao = i;
        bool destaque = i == app->selecao;
        DrawRectangleRounded(botao, 0.2f, 6, destaque ? (Color){74, 58, 36, 255} : Fade(COLOR_CARD, 0.85f));
        DrawRectangleRoundedLinesEx(botao, 0.2f, 6, destaque ? 2 : 1, destaque ? COLOR_AMBER : Fade(COLOR_EDGE, 0.4f));
        render_text(opcoes[i], botao.x + 22, botao.y + 14, 20, destaque ? COLOR_CREAM : Fade(COLOR_CREAM, 0.75f));
        if (ui_clicou(botao)) escolhida = i;
    }
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) escolhida = app->selecao;
    if (escolhida >= 0) audio_tocar(EFEITO_CLIQUE);
    return escolhida;
}

static void ir_para(App *app, Tela tela) {
    app->tela = tela;
    app->selecao = 0;
}

/* ---------- menu inicial ---------- */

static void comecar_campanha(App *app) {
    if (app->partida_ativa) game_shutdown(&app->game);
    game_init(&app->game);
    app->partida_ativa = true;
    ir_para(app, TELA_ABERTURA);
}

void tela_menu(App *app) {
    telas_desenhar_fundo(app->tempo);
    /* O título falha de vez em quando, como uma lâmpada com mau contato. */
    bool falha = fmodf(app->tempo, 7.3f) < 0.08f || fmodf(app->tempo, 11.1f) < 0.05f;
    render_text("ESTAÇÃO NOTURNA", 82, 132, 64, Fade(BLACK, 0.7f));
    render_text("ESTAÇÃO NOTURNA", 80, 130, 64, falha ? Fade(COLOR_CREAM, 0.25f) : COLOR_CREAM);
    render_text("SINAL DE RUPTURA", 84, 202, 24, COLOR_ALERT);
    render_text("104.7 · uma voz no escuro. Quatro noites. Em quem confiar?", 84, 244, 17, COLOR_MUTED);
    const char *const opcoes[] = {"Jogar", "Como jogar", "Configurações", "Sair"};
    int escolhida = telas_menu_vertical(app, opcoes, 4, 84, 320, 340);
    render_text("Setas + ENTER ou mouse", 84, 580, 13, Fade(COLOR_MUTED, 0.7f));
    if (escolhida == 0) comecar_campanha(app);
    if (escolhida == 1) { app->voltar_para = TELA_MENU; app->pagina_tutorial = 0; ir_para(app, TELA_TUTORIAL); }
    if (escolhida == 2) { app->voltar_para = TELA_MENU; ir_para(app, TELA_CONFIGURACOES); }
    if (escolhida == 3) ir_para(app, TELA_SAIR);
}

/* ---------- configurações ---------- */

void tela_configuracoes(App *app) {
    telas_desenhar_fundo(app->tempo);
    DrawRectangle(0, 0, VIRTUAL_WIDTH, VIRTUAL_HEIGHT, Fade(BLACK, 0.5f));
    render_text("CONFIGURAÇÕES", 240, 90, 36, COLOR_CREAM);
    Configuracoes *configuracoes = &app->configuracoes;
    float x = 240, largura = 800;
    bool mudou = false;
    mudou |= ui_deslizador((Rectangle){x, 170, largura, 56}, "Volume geral", &configuracoes->volume_geral);
    mudou |= ui_deslizador((Rectangle){x, 238, largura, 56}, "Música ambiente", &configuracoes->volume_musica);
    mudou |= ui_deslizador((Rectangle){x, 306, largura, 56}, "Efeitos sonoros", &configuracoes->volume_efeitos);
    mudou |= ui_alternador((Rectangle){x, 390, largura, 56}, "Tela cheia (F11)", &configuracoes->tela_cheia);
    mudou |= ui_alternador((Rectangle){x, 458, largura, 56}, "Dicas na tela (próxima tarefa)", &configuracoes->dicas);
    if (mudou) {
        configuracoes_salvar(configuracoes, ARQUIVO_CONFIGURACOES);
        audio_tocar(EFEITO_CONFIRMA);
    }
    render_text("As escolhas ficam salvas para a próxima vez.", x, 532, 14, COLOR_MUTED);
    if (ui_botao((Rectangle){x, 580, 220, 48}, "Voltar", true) || IsKeyPressed(KEY_ESCAPE)) ir_para(app, app->voltar_para);
}

/* ---------- abertura da noite ---------- */

void tela_abertura(App *app) {
    const Partida *partida = &app->game.partida;
    const DefinicaoNoite *noite = partida_definicao(partida);
    telas_desenhar_fundo(app->tempo);
    DrawRectangle(0, 0, VIRTUAL_WIDTH, VIRTUAL_HEIGHT, Fade(BLACK, 0.72f));
    render_text_centered(TextFormat("NOITE %d DE %d", partida->noite, TOTAL_NOITES), VIRTUAL_WIDTH / 2.0f, 200, 22, COLOR_ALERT);
    render_text_centered(noite->titulo, VIRTUAL_WIDTH / 2.0f, 236, 52, COLOR_CREAM);
    render_text_wrapped(noite->abertura, (Rectangle){290, 330, 700, 0}, 20, COLOR_MUTED);
    char fim[6];
    partida_formatar_hora(noite->pontos_acao, fim);
    render_text_centered(TextFormat("23:00 · a bateria aguenta até %s", fim), VIRTUAL_WIDTH / 2.0f, 470, 17, COLOR_AMBER);
    float alpha = 0.55f + 0.45f * sinf(app->tempo * 3.0f);
    render_text_centered("ENTER ou clique para entrar na cabine", VIRTUAL_WIDTH / 2.0f, 560, 18, Fade(COLOR_CREAM, alpha));
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        audio_tocar(EFEITO_MENSAGEM);
        game_start_night(&app->game);
        ir_para(app, TELA_JOGO);
    }
}

/* ---------- pausa ---------- */

void tela_pausa(App *app) {
    game_draw(&app->game, &app->configuracoes);
    DrawRectangle(0, 0, VIRTUAL_WIDTH, VIRTUAL_HEIGHT, Fade(BLACK, 0.72f));
    render_text("PAUSA", 84, 150, 44, COLOR_CREAM);
    char agora[6];
    partida_formatar_hora(app->game.partida.relogio, agora);
    render_text(TextFormat("Noite %d · %s · a chuva não para", app->game.partida.noite, agora),
                86, 204, 16, COLOR_MUTED);
    const char *const opcoes[] = {"Continuar", "Como jogar", "Configurações", "Abandonar a rádio"};
    int escolhida = telas_menu_vertical(app, opcoes, 4, 84, 260, 340);
    if (escolhida == 0 || IsKeyPressed(KEY_ESCAPE)) ir_para(app, TELA_JOGO);
    if (escolhida == 1) { app->voltar_para = TELA_PAUSA; app->pagina_tutorial = 0; ir_para(app, TELA_TUTORIAL); }
    if (escolhida == 2) { app->voltar_para = TELA_PAUSA; ir_para(app, TELA_CONFIGURACOES); }
    if (escolhida == 3) {
        game_shutdown(&app->game);
        app->partida_ativa = false;
        ir_para(app, TELA_MENU);
    }
}

/* ---------- final da campanha ---------- */

void tela_final(App *app) {
    const Partida *partida = &app->game.partida;
    const TextoFinal *final = campanha_final(partida->final);
    Color cor = partida->final == FINAL_COMPLETA ? COLOR_SIGNAL : partida->final == FINAL_CAPTURA ? COLOR_ALERT : COLOR_AMBER;
    telas_desenhar_fundo(app->tempo);
    DrawRectangle(0, 0, VIRTUAL_WIDTH, VIRTUAL_HEIGHT, Fade(BLACK, 0.55f));
    render_text_centered("FIM DA TRANSMISSÃO", VIRTUAL_WIDTH / 2.0f, 130, 20, COLOR_MUTED);
    render_text_centered(final->titulo, VIRTUAL_WIDTH / 2.0f, 164, 56, cor);
    render_text_wrapped(final->texto, (Rectangle){290, 270, 700, 0}, 21, COLOR_CREAM);
    render_text_centered(TextFormat("Provas que foram ao ar: %d de 4  ·  Noites resistidas: %d de 4", partida_total_provas(partida), partida->noite),
                         VIRTUAL_WIDTH / 2.0f, 460, 17, COLOR_MUTED);
    if (ui_botao((Rectangle){VIRTUAL_WIDTH / 2.0f - 140, 540, 280, 52}, "Voltar ao menu", true) || IsKeyPressed(KEY_ENTER)) {
        game_shutdown(&app->game);
        app->partida_ativa = false;
        ir_para(app, TELA_MENU);
    }
}
