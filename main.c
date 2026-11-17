#include "interface.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

/* Telas do jogo (menu, tutorial, configurações, abertura, pausa, resumo e final) e a main. */
#define ARQUIVO_CONFIGURACOES "settings.cfg"

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
    Tela voltar_para;
    GameState game;
    bool partida_ativa;
    Configuracoes configuracoes;
    int selecao;
    int pagina_tutorial;
    float tempo;
} App;

/* ---------- configurações salvas em settings.cfg ---------- */
static Configuracoes configuracoes_padrao(void) {
    return (Configuracoes){.volume_geral = 80, .volume_musica = 60, .volume_efeitos = 80, .tela_cheia = false, .dicas = true};
}

static int limitar(int valor, int padrao) { return (valor < 0 || valor > 100) ? padrao : valor; }

static void aplicar_linha(Configuracoes *configuracoes, const char *chave, int valor) {
    Configuracoes padrao = configuracoes_padrao();
    if (strcmp(chave, "volume_geral") == 0) configuracoes->volume_geral = limitar(valor, padrao.volume_geral);
    if (strcmp(chave, "volume_musica") == 0) configuracoes->volume_musica = limitar(valor, padrao.volume_musica);
    if (strcmp(chave, "volume_efeitos") == 0) configuracoes->volume_efeitos = limitar(valor, padrao.volume_efeitos);
    if (strcmp(chave, "tela_cheia") == 0) configuracoes->tela_cheia = valor != 0;
    if (strcmp(chave, "dicas") == 0) configuracoes->dicas = valor != 0;
}

static Configuracoes configuracoes_carregar(const char *caminho) {
    Configuracoes configuracoes = configuracoes_padrao();
    FILE *arquivo = fopen(caminho, "r");
    if (!arquivo) return configuracoes;
    char chave[64];
    int valor;
    while (fscanf(arquivo, " %63[^=]=%d", chave, &valor) == 2) aplicar_linha(&configuracoes, chave, valor);
    fclose(arquivo);
    return configuracoes;
}

static bool configuracoes_salvar(const Configuracoes *configuracoes, const char *caminho) {
    FILE *arquivo = fopen(caminho, "w");
    if (!arquivo) return false;
    fprintf(arquivo, "volume_geral=%d\nvolume_musica=%d\nvolume_efeitos=%d\ntela_cheia=%d\ndicas=%d\n",
            configuracoes->volume_geral, configuracoes->volume_musica, configuracoes->volume_efeitos,
            configuracoes->tela_cheia ? 1 : 0, configuracoes->dicas ? 1 : 0);
    fclose(arquivo);
    return true;
}

/* ---------- fundo dos menus: cidade, chuva e holofote ---------- */
#define CICLO_RELAMPAGO_MENU 29.0f
#define HORIZONTE 196

static const Vector2 TOPO_DA_TORRE = {318, 74};

static float relampago(float tempo) {
    float fase = fmodf(tempo + 4.0f, CICLO_RELAMPAGO_MENU);
    if (fase < 0.1f) return 1.0f;
    if (fase > 0.18f && fase < 0.26f) return 0.6f;
    return 0.0f;
}

static void trovao(float tempo) {
    static float tempo_anterior = 0;
    float antes = fmodf(tempo_anterior + 4.0f, CICLO_RELAMPAGO_MENU), agora = fmodf(tempo + 4.0f, CICLO_RELAMPAGO_MENU);
    if (antes < 1.3f && agora >= 1.3f) audio_tocar(EFEITO_TROVAO);
    tempo_anterior = tempo;
}

static void ceu(float clarao) {
    const Color topo = {8, 9, 18, 255}, base = {34, 26, 44, 255};
    for (int y = 0; y < HORIZONTE; y++) {
        float t = (float)y / HORIZONTE;
        Color linha = ColorLerp(ColorLerp(topo, base, t), (Color){150, 160, 200, 255}, clarao * 0.6f);
        px_hlinha(0, y, MUNDO_LARGURA, linha);
        if (y % 2 == 0 && px_ruido(y, 9) % 3 == 0) {
            for (int x = (int)(px_ruido(y, 1) % 7); x < MUNDO_LARGURA; x += 7) px_ponto(x, y, px_escurecer(linha, 0.8f));
        }
    }
}

static void janelas_do_predio(int x, int topo, int largura, int predio, float tempo) {
    for (int y = topo + 4; y < MUNDO_ALTURA - 4; y += 7) {
        for (int coluna = x + 3; coluna < x + largura - 3; coluna += 6) {
            unsigned ruido = px_ruido(coluna, y + predio);
            if (ruido % 9 != 0) continue;
            bool pisca = ruido % 5 == 0 && sinf(tempo * 0.7f + (float)(ruido % 13)) < -0.7f;
            if (!pisca) px_ret(coluna, y, 2, 3, ruido % 4 == 0 ? (Color){120, 170, 150, 255} : (Color){214, 160, 80, 255});
        }
    }
}

static void predios(float tempo) {
    const int alturas[] = {70, 110, 52, 132, 88, 64, 120, 58, 98, 80, 140, 72, 104};
    const int larguras[] = {34, 30, 40, 28, 36, 30, 34, 38, 30, 36, 26, 34, 32};
    int x = -6;
    for (int i = 0; i < 13; i++) {
        int topo = MUNDO_ALTURA - alturas[i];
        px_ret(x, topo, larguras[i], alturas[i], (Color){12, 13, 20, 255});
        px_hlinha(x, topo, larguras[i], (Color){30, 30, 42, 255});
        janelas_do_predio(x, topo, larguras[i], i, tempo);
        x += larguras[i] + 1;
    }
}

static void torre(float tempo) {
    int x = (int)TOPO_DA_TORRE.x, y = (int)TOPO_DA_TORRE.y;
    DrawLine(x - 14, MUNDO_ALTURA, x, y, (Color){34, 34, 46, 255});
    DrawLine(x + 14, MUNDO_ALTURA, x, y, (Color){34, 34, 46, 255});
    for (int degrau = y + 10; degrau < MUNDO_ALTURA; degrau += 12) {
        int abertura = (degrau - y) * 14 / (MUNDO_ALTURA - y);
        DrawLine(x - abertura, degrau, x + abertura, degrau + 12, (Color){28, 28, 38, 255});
    }
    px_ret(x - 3, y - 2, 7, 4, (Color){40, 40, 52, 255});
    if (sinf(tempo * 2.4f) > 0) px_ret(x - 1, y - 5, 2, 2, (Color){240, 70, 60, 255});
}

static void holofote(float tempo) {
    float angulo = sinf(tempo * 0.35f) * 1.1f - PI / 2;
    Vector2 ponta = {TOPO_DA_TORRE.x + cosf(angulo) * 420, TOPO_DA_TORRE.y + sinf(angulo) * 420};
    BeginBlendMode(BLEND_ADDITIVE);
    const float aberturas[] = {80.0f, 36.0f}, forcas[] = {0.07f, 0.10f};
    for (int camada = 0; camada < 2; camada++) {
        Vector2 lateral = {-sinf(angulo) * aberturas[camada], cosf(angulo) * aberturas[camada]};
        Vector2 a = {ponta.x + lateral.x, ponta.y + lateral.y}, b = {ponta.x - lateral.x, ponta.y - lateral.y};
        Color luz = Fade((Color){255, 246, 210, 255}, forcas[camada]);
        DrawTriangle(TOPO_DA_TORRE, a, b, luz);
        DrawTriangle(TOPO_DA_TORRE, b, a, luz);
    }
    EndBlendMode();
}

static void chuva(float tempo) {
    for (int gota = 0; gota < 90; gota++) {
        unsigned ruido = px_ruido(gota, 21);
        float velocidade = 150.0f + (float)(ruido % 60);
        int x = (int)((ruido >> 4) % MUNDO_LARGURA) - (int)(fmodf(tempo * 30.0f, 40.0f));
        int y = (int)fmodf(tempo * velocidade + (float)(ruido % 400), (float)MUNDO_ALTURA + 20) - 10;
        DrawLine(x, y, x - 1, y + 4, (Color){90, 104, 140, 160});
    }
}

static void telas_desenhar_fundo(float tempo) {
    float clarao = relampago(tempo);
    trovao(tempo);
    pixel_mundo_comecar(BLACK);
    ceu(clarao);
    holofote(tempo);
    torre(tempo);
    predios(tempo);
    chuva(tempo);
    pixel_vinheta(1.0f);
    pixel_mundo_terminar();
    DrawRectangleGradientH(0, 0, 700, VIRTUAL_HEIGHT, Fade(BLACK, 0.75f), Fade(BLACK, 0));
}

/* ---------- telas ----------
   Menu vertical que funciona com setas + ENTER ou mouse; devolve o índice escolhido ou -1. */
static int telas_menu_vertical(App *app, const char *const opcoes[], int total, float x, float y, float largura) {
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

static void comecar_campanha(App *app) {
    if (app->partida_ativa) game_shutdown(&app->game);
    game_init(&app->game);
    app->partida_ativa = true;
    ir_para(app, TELA_ABERTURA);
}

static void tela_menu(App *app) {
    telas_desenhar_fundo(app->tempo);
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

static void tela_configuracoes(App *app) {
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

static void tela_abertura(App *app) {
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

static void tela_pausa(App *app) {
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

static void tela_final(App *app) {
    const Partida *partida = &app->game.partida;
    const TextoFinal *final = campanha_final(partida->final);
    Color cor = partida->final == FINAL_COMPLETA ? COLOR_SIGNAL : partida->final == FINAL_CAPTURA ? COLOR_ALERT : COLOR_AMBER;
    telas_desenhar_fundo(app->tempo);
    DrawRectangle(0, 0, VIRTUAL_WIDTH, VIRTUAL_HEIGHT, Fade(BLACK, 0.55f));
    render_text_centered("FIM DA TRANSMISSÃO", VIRTUAL_WIDTH / 2.0f, 130, 20, COLOR_MUTED);
    render_text_centered(final->titulo, VIRTUAL_WIDTH / 2.0f, 164, 56, cor);
    render_text_wrapped(final->texto, (Rectangle){290, 270, 700, 0}, 21, COLOR_CREAM);
    int noites_resistidas = partida->final == FINAL_CAPTURA ? partida->noite - 1 : partida->noite;
    render_text_centered(TextFormat("Provas que foram ao ar: %d de 4  ·  Noites resistidas: %d de %d", partida_total_provas(partida), noites_resistidas, TOTAL_NOITES),
                         VIRTUAL_WIDTH / 2.0f, 460, 17, COLOR_MUTED);
    if (ui_botao((Rectangle){VIRTUAL_WIDTH / 2.0f - 140, 540, 280, 52}, "Voltar ao menu", true) || IsKeyPressed(KEY_ENTER)) {
        game_shutdown(&app->game);
        app->partida_ativa = false;
        ir_para(app, TELA_MENU);
    }
}

typedef struct {
    const char *titulo;
    const char *texto;
} PaginaTutorial;

/* Páginas do tutorial "Como jogar". */
static const PaginaTutorial PAGINAS[] = {
    {"A RÁDIO",
     "O antigo operador sumiu, e a rádio clandestina da frequência 104.7 agora é sua. Durante quatro noites, você "
     "recebe mensagens da rede, decide em quem acreditar e escolhe o que vai ao ar, e em que ordem.\n\n"
     "Proteja quem precisa de ajuda, junte provas contra o regime e, na última noite, ponha a denúncia no ar antes "
     "que batam à sua porta."},
    {"CONTROLES",
     "WASD ou setas: andar pela cabine.\nE: usar o que estiver perto. E ou ESC fecham o que estiver aberto.\n"
     "TAB: tarefas da noite.   ESC: pausa.   F11: tela cheia.\n\n"
     "No transmissor: Cima/Baixo escolhem, Shift+Cima/Baixo mudam a ordem, P prioriza os urgentes, R tira do "
     "roteiro e ENTER transmite. Tudo também funciona com o mouse."},
    {"UMA NOITE",
     "Antes de ligar a rádio, o tempo não corre: leia as novidades no receptor, consulte os papéis e escolha na "
     "bancada como se preparar.\n\nDepois que a rádio liga, o relógio anda a cada decisão importante: pedir confirmação, "
     "investigar, transmitir, trocar de frequência, mexer na cortina. Cada uma consome 15 minutos ou mais de bateria. "
     "Andar, ler, comparar e reorganizar não custam nada.\n\nA noite termina quando a bateria acaba ou quando você vai dormir."},
    {"MENSAGENS",
     "Os sinais chegam ao receptor um de cada vez, na ordem em que foram captados. Nem tudo o que chega é verdade.\n\n"
     "Na bancada, compare uma mensagem com os documentos: uma senha antiga, um horário impossível ou uma informação "
     "que ninguém deveria ter entregam a mentira. Pedir confirmação à rede resolve a dúvida, mas custa tempo. "
     "Investigar traz documentos novos.\n\nO que merecer ir ao ar, ponha no roteiro."},
    {"O ROTEIRO",
     "No transmissor fica o roteiro da noite: o primeiro da lista é o próximo a ir ao ar.\n\n"
     "Cada aviso só serve até um certo horário, e muitos só são levados a sério se a senha da noite for ao ar antes. "
     "\"Priorizar urgentes\" coloca primeiro o que tem menos tempo, mas não pensa nas senhas: confira a ordem e ajuste "
     "com Subir e Descer.\n\nCada transmissão deixa um rastro no ar."},
    {"PERIGOS",
     "SINAL EXPOSTO: se o rastro ficar forte demais, a polícia te encontra e a noite acaba. Trocar de frequência ajuda.\n"
     "SUSPEITA: cresce com erros, armadilhas e descuidos, e pesa quando os agentes chegarem.\n\n"
     "HOLOFOTE: quando a sirene tocar, uma área do chão fica marcada. Feche a cortina ou não esteja ali quando a luz passar.\n"
     "REVISTA: na última noite, batem à sua porta. Esconda os papéis e desligue tudo antes de atender.\n\n"
     "E desconfie de quem tem pressa."},
};
#define TOTAL_PAGINAS ((int)(sizeof(PAGINAS) / sizeof(PAGINAS[0])))

static void mudar_pagina(App *app, int direcao) {
    int nova = app->pagina_tutorial + direcao;
    if (nova < 0 || nova >= TOTAL_PAGINAS) return;
    app->pagina_tutorial = nova;
    audio_tocar(EFEITO_PAPEL);
}

static void desenhar_indicador(int atual) {
    for (int i = 0; i < TOTAL_PAGINAS; i++) {
        Vector2 centro = {VIRTUAL_WIDTH / 2.0f - (TOTAL_PAGINAS - 1) * 12 + i * 24, 600};
        DrawCircleV(centro, i == atual ? 6 : 4, i == atual ? COLOR_AMBER : Fade(COLOR_MUTED, 0.6f));
    }
}

static void tela_tutorial(App *app) {
    telas_desenhar_fundo(app->tempo);
    DrawRectangle(0, 0, VIRTUAL_WIDTH, VIRTUAL_HEIGHT, Fade(BLACK, 0.55f));
    Rectangle folha = {190, 70, 900, 500};
    DrawRectangleRec(folha, COLOR_PANEL);
    DrawRectangleLinesEx(folha, 3, COLOR_EDGE);
    const PaginaTutorial *pagina = &PAGINAS[app->pagina_tutorial];
    render_text(TextFormat("COMO JOGAR %d/%d", app->pagina_tutorial + 1, TOTAL_PAGINAS), folha.x + 36, folha.y + 26, 14, COLOR_MUTED);
    render_text(pagina->titulo, folha.x + 36, folha.y + 48, 30, COLOR_AMBER);
    render_text_wrapped(pagina->texto, (Rectangle){folha.x + 36, folha.y + 104, folha.width - 72, 0}, 18, COLOR_CREAM);
    desenhar_indicador(app->pagina_tutorial);
    if (ui_botao((Rectangle){190, 630, 180, 46}, "< Anterior", app->pagina_tutorial > 0) || IsKeyPressed(KEY_LEFT)) mudar_pagina(app, -1);
    if (ui_botao((Rectangle){910, 630, 180, 46}, "Próxima >", app->pagina_tutorial < TOTAL_PAGINAS - 1) || IsKeyPressed(KEY_RIGHT)) mudar_pagina(app, 1);
    if (ui_botao((Rectangle){550, 630, 180, 46}, "Voltar", true) || IsKeyPressed(KEY_ESCAPE)) {
        app->tela = app->voltar_para;
        app->selecao = 0;
    }
}

/* ---------- resumo do fim da noite ---------- */
static const char *motivo_do_fim(MotivoFim motivo) {
    switch (motivo) {
        case FIM_DORMIU: return "Você desligou a rádio e tentou dormir.";
        case FIM_SEM_PONTOS: return "A bateria morreu antes do amanhecer.";
        case FIM_RASTREADO: return "Um carro de escuta achou o sinal. Você precisou arrancar os cabos.";
        case FIM_CAPTURA: return "Os agentes encontraram a rádio.";
        default: return "";
    }
}

static float desenhar_transmissoes(const Partida *partida, float x, float y, float largura) {
    render_text("O QUE FOI AO AR", x, y, 14, COLOR_AMBER);
    y += 24;
    if (partida->total_registros == 0) {
        render_text("A frequência ficou em silêncio esta noite.", x, y, 15, COLOR_MUTED);
        return y + 26;
    }
    for (int i = 0; i < partida->total_registros; i++) {
        const RegistroTransmissao *registro = &partida->registros[i];
        render_text(registro->sucesso ? "OK" : "X", x, y, 15, registro->sucesso ? COLOR_SIGNAL : COLOR_ALERT);
        render_text(registro->titulo, x + 30, y, 15, COLOR_CREAM);
        y += 20 + render_text_wrapped(registro->resultado, (Rectangle){x + 30, y + 20, largura - 30, 0}, 13, COLOR_MUTED) + 4;
    }
    return y;
}

static float desenhar_tarefas(const Partida *partida, float x, float y) {
    render_text("TAREFAS DA NOITE", x, y, 14, COLOR_AMBER);
    y += 24;
    int total;
    const Tarefa *tarefas = campanha_tarefas(&total);
    for (int i = 0; i < total; i++) {
        if (!partida_tarefa_visivel(partida, &tarefas[i])) continue;
        bool feita = partida_tarefa_concluida(partida, &tarefas[i]);
        Color cor = feita ? COLOR_SIGNAL : tarefas[i].obrigatoria ? COLOR_ALERT : COLOR_MUTED;
        render_text(feita ? "OK" : "X", x, y, 14, cor);
        render_text(TextFormat("%s%s", tarefas[i].texto, tarefas[i].obrigatoria ? "" : " (opcional)"), x + 30, y, 14, feita ? COLOR_CREAM : cor);
        y += 21;
    }
    return y;
}

static float desenhar_consequencias(const Partida *partida, float x, float y, float largura) {
    if (partida->total_consequencias == 0) return y;
    render_text("O PREÇO DA NOITE", x, y, 14, COLOR_ALERT);
    y += 24;
    for (int i = 0; i < partida->total_consequencias; i++) {
        y += render_text_wrapped(TextFormat("- %s", partida->consequencias[i]), (Rectangle){x, y, largura, 0}, 14, COLOR_CREAM) + 4;
    }
    return y + 10;
}

static void linha_estado(float x, float *y, const char *rotulo, const char *valor, Color cor) {
    render_text(rotulo, x, *y, 14, COLOR_MUTED);
    render_text(valor, x + 170, *y, 14, cor);
    *y += 22;
}

static void desenhar_situacao(const Partida *partida, float x, float y) {
    render_text("A REDE", x, y, 14, COLOR_AMBER);
    y += 24;
    unsigned marcas = partida->marcas;
    linha_estado(x, &y, "Lince", (marcas & MARCA_LINCE_AJUDADO) ? "a salvo, devendo um favor" : "sem notícias", (marcas & MARCA_LINCE_AJUDADO) ? COLOR_SIGNAL : COLOR_MUTED);
    linha_estado(x, &y, "Garça", (marcas & MARCA_GARCA_AJUDADA) ? "a salvo, devendo um favor" : "na escuta", (marcas & MARCA_GARCA_AJUDADA) ? COLOR_SIGNAL : COLOR_CREAM);
    const char *farol = (marcas & MARCA_FAROL_DESMASCARADO) ? "desmascarado" : (marcas & MARCA_FAROL_COM_PROGRAMACAO) ? "conhece o seu roteiro" : "mantido à distância";
    linha_estado(x, &y, "Farol", farol, (marcas & MARCA_FAROL_DESMASCARADO) ? COLOR_SIGNAL : COLOR_CREAM);
    linha_estado(x, &y, "Provas no ar", TextFormat("%d de 4", partida_total_provas(partida)), COLOR_CREAM);
    linha_estado(x, &y, "Desconfiança", partida->suspeita < 25 ? "baixa" : partida->suspeita < 50 ? "média" : "alta",
                 partida->suspeita >= 50 ? COLOR_ALERT : COLOR_CREAM);
    y += 8;
    render_text("O QUE VOCÊ GANHOU", x, y, 13, COLOR_AMBER);
    y += 22;
    if (marcas & MARCA_BENEFICIO_AVISO) { render_text("- O Lince avisa antes quando a patrulha vier", x, y, 13, COLOR_SIGNAL); y += 20; }
    if (marcas & MARCA_BENEFICIO_CONFIRMACAO) { render_text("- A Garça confirma uma mensagem por noite, sem custo", x, y, 13, COLOR_SIGNAL); y += 20; }
    if (marcas & MARCA_BENEFICIO_ESCONDERIJO) { render_text("- Um fundo falso no piso para esconder papéis", x, y, 13, COLOR_SIGNAL); y += 20; }
    if (!(marcas & (MARCA_BENEFICIO_AVISO | MARCA_BENEFICIO_CONFIRMACAO | MARCA_BENEFICIO_ESCONDERIJO))) render_text("Nada ainda.", x, y, 13, COLOR_MUTED);
}

static void avancar(App *app) {
    audio_tocar(EFEITO_CONFIRMA);
    app->selecao = 0;
    if (partida_proxima_noite(&app->game.partida)) app->tela = TELA_ABERTURA;
    else app->tela = TELA_FINAL;
}

static void tela_resumo(App *app) {
    const Partida *partida = &app->game.partida;
    ClearBackground(COLOR_NIGHT);
    render_text(TextFormat("FIM DA NOITE %d · %s", partida->noite, partida_definicao(partida)->titulo), 60, 36, 30, COLOR_CREAM);
    render_text(motivo_do_fim(partida->motivo_fim), 62, 76, 16, partida->motivo_fim == FIM_CAPTURA ? COLOR_ALERT : COLOR_MUTED);
    float y = desenhar_transmissoes(partida, 60, 118, 560);
    desenhar_consequencias(partida, 60, y + 14, 560);
    float y_direita = desenhar_tarefas(partida, 680, 118);
    desenhar_situacao(partida, 680, y_direita + 20);
    bool acabou = partida->final != FINAL_NENHUM || partida->noite >= TOTAL_NOITES;
    const char *rotulo = acabou ? "Amanhecer" : "Próxima noite";
    if (ui_botao((Rectangle){VIRTUAL_WIDTH - 300, VIRTUAL_HEIGHT - 76, 240, 50}, rotulo, true) || IsKeyPressed(KEY_ENTER)) avancar(app);
}

/* ---------- laço principal ---------- */
static void app_iniciar(App *app) {
    app->tela = TELA_MENU;
    app->voltar_para = TELA_MENU;
    app->partida_ativa = false;
    app->configuracoes = configuracoes_carregar(ARQUIVO_CONFIGURACOES);
    app->selecao = 0;
    app->pagina_tutorial = 0;
    app->tempo = 0.0f;
    ui_definir_som_clique(audio_tocar_clique);
}

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

/* Atualiza e desenha a tela atual; chamada uma vez por quadro. */
static void app_quadro(App *app) {
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

static void app_encerrar(App *app) {
    if (app->partida_ativa) game_shutdown(&app->game);
    configuracoes_salvar(&app->configuracoes, ARQUIVO_CONFIGURACOES);
}

static bool app_deve_sair(const App *app) { return app->tela == TELA_SAIR; }

/* Abre a janela, carrega gráficos e sons e roda o laço até o jogador sair. */
int main(void) {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);
    InitWindow(VIRTUAL_WIDTH, VIRTUAL_HEIGHT, "Estação Noturna: Sinal de Ruptura");
    SetWindowMinSize(640, 360);
    SetExitKey(KEY_NULL);
    SetTargetFPS(60);
    render_init();
    pixel_iniciar();
    sprites_iniciar();
    cabin_iniciar();
    audio_iniciar();

    App app;
    app_iniciar(&app);
    while (!WindowShouldClose() && !app_deve_sair(&app)) {
        render_begin_frame();
        app_quadro(&app);
        render_end_frame();
    }
    app_encerrar(&app);
    audio_encerrar();
    cabin_encerrar();
    sprites_encerrar();
    pixel_encerrar();
    render_shutdown();
    CloseWindow();
    return 0;
}
