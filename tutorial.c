/* Tutorial em páginas: objetivo, controles, fases, investigação, programação e perigos. */
#include "app.h"
#include "audio.h"
#include "raylib.h"
#include "render.h"
#include "widgets.h"

typedef struct {
    const char *titulo;
    const char *texto;
} PaginaTutorial;

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

void tela_tutorial(App *app) {
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
