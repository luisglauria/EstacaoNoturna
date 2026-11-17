/* Encerramento da noite: transmissões, tarefas, consequências, contatos, provas e recursos. */
#include "app.h"
#include "audio.h"
#include "raylib.h"
#include "render.h"
#include "widgets.h"

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

void tela_resumo(App *app) {
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
