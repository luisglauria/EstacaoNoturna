#include "hud.h"
#include "render.h"
#include "widgets.h"
#include <math.h>

/* ---------- topo ---------- */

static void desenhar_fase(const Partida *partida, float x, float y) {
    bool no_ar = partida->fase == FASE_OPERACAO;
    const char *fase = partida->fase == FASE_PREPARACAO ? "PREPARAÇÃO" : no_ar ? "NO AR" : "ENCERRADA";
    ui_etiqueta(x, y, fase, no_ar ? COLOR_ALERT : COLOR_SIGNAL);
    const char *detalhe = partida->fase == FASE_PREPARACAO ? "o tempo ainda não corre" : "";
    render_text(detalhe, x + render_text_width(fase, 12) + 26, y + 4, 13, COLOR_MUTED);
}

static void desenhar_relogio(const Partida *partida, float x) {
    char agora[6], fim[6];
    partida_formatar_hora(partida->relogio, agora);
    partida_formatar_hora(partida->relogio + partida->pontos_acao, fim);
    render_text(agora, x, 6, 30, partida->fase == FASE_OPERACAO ? COLOR_CREAM : COLOR_MUTED);
    render_text(TextFormat("bateria até %s", fim), x + 2, 40, 12, COLOR_MUTED);
}

static void desenhar_bateria(const Partida *partida, float x) {
    render_text("BATERIA", x, 8, 11, COLOR_MUTED);
    for (int i = 0; i < partida->pontos_maximos; i++) {
        Rectangle celula = {x + i * 13, 28, 9, 16};
        if (i < partida->pontos_acao) DrawRectangleRec(celula, partida->pontos_acao <= 2 ? COLOR_ALERT : COLOR_AMBER);
        else DrawRectangleLinesEx(celula, 1, Fade(COLOR_AMBER, 0.35f));
    }
}

void hud_desenhar_topo(const Partida *partida) {
    DrawRectangle(0, 0, VIRTUAL_WIDTH, HUD_ALTURA_TOPO, COLOR_BAR);
    DrawLine(0, HUD_ALTURA_TOPO, VIRTUAL_WIDTH, HUD_ALTURA_TOPO, Fade(COLOR_EDGE, 0.4f));
    const DefinicaoNoite *noite = partida_definicao(partida);
    render_text(TextFormat("NOITE %d · %s", partida->noite, noite->titulo), 24, 8, 18, COLOR_CREAM);
    desenhar_fase(partida, 24, 32);
    desenhar_relogio(partida, 400);
    desenhar_bateria(partida, 520);
    ui_medidor((Rectangle){700, 10, 160, 38}, "SINAL EXPOSTO", partida->rastreamento, partida->rastreamento >= 70 ? COLOR_ALERT : COLOR_SIGNAL);
    ui_medidor((Rectangle){886, 10, 160, 38}, "SUSPEITA", partida->suspeita, partida->suspeita >= 50 ? COLOR_ALERT : COLOR_AMBER);
    render_text("PROVAS", 1078, 8, 11, COLOR_MUTED);
    render_text(TextFormat("%d", partida_total_provas(partida)), 1078, 24, 22, COLOR_CREAM);
    render_text("SINAIS", 1160, 8, 11, COLOR_MUTED);
    render_text(TextFormat("%d", partida->fila.tamanho), 1160, 24, 22, partida->fila.tamanho ? COLOR_SIGNAL : COLOR_MUTED);
}

/* ---------- rodapé, alerta e dica ---------- */

static void contar_obrigatorias(const Partida *partida, int *feitas, int *total) {
    int quantidade;
    const Tarefa *tarefas = campanha_tarefas(&quantidade);
    *feitas = *total = 0;
    for (int i = 0; i < quantidade; i++) {
        if (!tarefas[i].obrigatoria || !partida_tarefa_visivel(partida, &tarefas[i])) continue;
        (*total)++;
        if (partida_tarefa_concluida(partida, &tarefas[i])) (*feitas)++;
    }
}

void hud_desenhar_rodape(const Partida *partida, bool painel_aberto) {
    DrawRectangle(0, HUD_Y_RODAPE, VIRTUAL_WIDTH, VIRTUAL_HEIGHT - HUD_Y_RODAPE, COLOR_BAR);
    DrawLine(0, HUD_Y_RODAPE, VIRTUAL_WIDTH, HUD_Y_RODAPE, Fade(COLOR_EDGE, 0.3f));
    int feitas, total;
    contar_obrigatorias(partida, &feitas, &total);
    const Tarefa *proxima = partida_proxima_obrigatoria(partida);
    render_text(TextFormat("OBRIGATÓRIAS %d/%d", feitas, total), 24, HUD_Y_RODAPE + 10, 14, feitas == total ? COLOR_SIGNAL : COLOR_AMBER);
    const char *texto = proxima ? proxima->texto : "O essencial está feito. Cuide das opcionais ou vá dormir.";
    render_text(TextFormat("> %s", texto), 210, HUD_Y_RODAPE + 10, 14, COLOR_CREAM);
    if (painel_aberto) {
        ui_dica_tecla("ESC", "fechar", 1150, HUD_Y_RODAPE + 7);
        return;
    }
    float x = 900;
    x += ui_dica_tecla("E", "usar", x, HUD_Y_RODAPE + 7);
    x += ui_dica_tecla("TAB", "tarefas", x, HUD_Y_RODAPE + 7);
    ui_dica_tecla("ESC", "pausa", x, HUD_Y_RODAPE + 7);
}

void hud_desenhar_alerta(const Partida *partida, float tempo) {
    int holofote = partida_acoes_ate_holofote(partida), inspecao = partida_acoes_ate_inspecao(partida);
    if (holofote < 0 && inspecao < 0) return;
    char hora[6];
    partida_formatar_hora(inspecao >= 0 ? partida->inspecao_chegada : partida->holofote_chegada, hora);
    const char *texto = inspecao >= 0 ? TextFormat("REVISTA ÀS %s · esconda os papéis, desligue tudo e atenda a porta", hora)
                                      : TextFormat("HOLOFOTE ÀS %s · feche a cortina ou fique fora da luz", hora);
    float largura = render_text_width(texto, 15) + 40;
    Rectangle faixa = {(VIRTUAL_WIDTH - largura) / 2, HUD_ALTURA_TOPO + 8, largura, 30};
    float pulso = 0.6f + 0.4f * sinf(tempo * 5.0f);
    DrawRectangleRec(faixa, Fade((Color){60, 10, 12, 255}, 0.92f));
    DrawRectangleLinesEx(faixa, 1, Fade(COLOR_ALERT, pulso));
    render_text_centered(texto, VIRTUAL_WIDTH / 2.0f, faixa.y + 7, 15, COLOR_CREAM);
}

void hud_desenhar_dica(const Partida *partida) {
    const Tarefa *proxima = partida_proxima_obrigatoria(partida);
    if (!proxima || !proxima->dica || !proxima->dica[0]) return;
    float largura = render_text_width(proxima->dica, 14) + 30;
    Rectangle caixa = {(VIRTUAL_WIDTH - largura) / 2, HUD_Y_RODAPE - 38, largura, 28};
    DrawRectangleRec(caixa, Fade(COLOR_BAR, 0.85f));
    DrawRectangleLinesEx(caixa, 1, Fade(COLOR_SIGNAL, 0.45f));
    render_text(proxima->dica, caixa.x + 15, caixa.y + 6, 14, Fade(COLOR_CREAM, 0.9f));
}

/* ---------- lista de tarefas (TAB) ---------- */

static float desenhar_secao(const Partida *partida, bool obrigatorias, float x, float y) {
    render_text(obrigatorias ? "OBRIGATÓRIAS" : "OPCIONAIS", x, y, 15, obrigatorias ? COLOR_AMBER : COLOR_SIGNAL);
    y += 28;
    int quantidade;
    const Tarefa *tarefas = campanha_tarefas(&quantidade);
    for (int i = 0; i < quantidade; i++) {
        const Tarefa *tarefa = &tarefas[i];
        if (tarefa->obrigatoria != obrigatorias || !partida_tarefa_visivel(partida, tarefa)) continue;
        bool feita = partida_tarefa_concluida(partida, tarefa);
        DrawRectangleLinesEx((Rectangle){x, y + 1, 16, 16}, 1, feita ? COLOR_SIGNAL : COLOR_MUTED);
        if (feita) DrawRectangle((int)x + 4, (int)y + 5, 8, 8, COLOR_SIGNAL);
        render_text(tarefa->texto, x + 30, y, 16, feita ? Fade(COLOR_MUTED, 0.8f) : COLOR_CREAM);
        if (feita) DrawLine((int)x + 30, (int)y + 9, (int)(x + 30 + render_text_width(tarefa->texto, 16)), (int)y + 9, Fade(COLOR_MUTED, 0.7f));
        y += 28;
    }
    return y;
}

void hud_desenhar_tarefas(const Partida *partida) {
    DrawRectangle(0, 0, VIRTUAL_WIDTH, VIRTUAL_HEIGHT, Fade(BLACK, 0.6f));
    Rectangle painel = {250, 96, 780, 540};
    DrawRectangleRec(painel, COLOR_PANEL);
    DrawRectangleLinesEx(painel, 2, COLOR_EDGE);
    render_text(TextFormat("TAREFAS DA NOITE %d", partida->noite), painel.x + 30, painel.y + 22, 24, COLOR_CREAM);
    render_text("O que ficar pendente entre as obrigatórias terá consequências ao amanhecer.", painel.x + 30, painel.y + 58, 14, COLOR_MUTED);
    float y = desenhar_secao(partida, true, painel.x + 30, painel.y + 96);
    desenhar_secao(partida, false, painel.x + 30, y + 18);
    ui_dica_tecla("TAB", "fechar", painel.x + painel.width - 130, painel.y + painel.height - 38);
}

/* ---------- balão e aviso ---------- */

void hud_desenhar_balao(Vector2 posicao, const char *rotulo, float tempo) {
    float largura = render_text_width(rotulo, 14) + 46;
    float balanco = sinf(tempo * 3.0f) * 2.0f;
    Rectangle balao = {posicao.x - largura / 2, posicao.y - 100 + balanco, largura, 30};
    DrawRectangleRec(balao, Fade(COLOR_BAR, 0.9f));
    DrawRectangleLinesEx(balao, 1, Fade(COLOR_AMBER, 0.7f));
    ui_tecla("E", balao.x + 6, balao.y + 4);
    render_text(rotulo, balao.x + 36, balao.y + 7, 14, COLOR_CREAM);
}

void hud_desenhar_aviso(const char *texto, float alpha, bool em_painel) {
    if (!texto[0] || alpha <= 0.0f) return;
    if (em_painel) {
        DrawRectangle(180, 606, 6, 6, Fade(COLOR_AMBER, alpha));
        render_text_wrapped(texto, (Rectangle){198, 601, 900, 0}, 14, Fade(COLOR_AMBER, alpha));
        return;
    }
    float largura = 820;
    float altura = render_text_wrapped_height(texto, largura - 40, 15) + 24;
    Rectangle caixa = {(VIRTUAL_WIDTH - largura) / 2, HUD_Y_RODAPE - 50 - altura, largura, altura};
    DrawRectangleRec(caixa, Fade((Color){8, 9, 13, 255}, 0.93f * alpha));
    DrawRectangleLinesEx(caixa, 1, Fade(COLOR_EDGE, alpha));
    render_text_wrapped(texto, (Rectangle){caixa.x + 20, caixa.y + 12, largura - 40, altura}, 15, Fade(COLOR_CREAM, alpha));
}
