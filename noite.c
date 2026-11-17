#include "interface.h"
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

/* A noite na tela: HUD, painéis de cada objeto e o laço da noite. */

/* ---------- HUD (topo, rodapé, tarefas e avisos) ---------- */
#define HUD_ALTURA_TOPO 60
#define HUD_Y_RODAPE 684

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

static void hud_desenhar_topo(const Partida *partida) {
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

static void hud_desenhar_rodape(const Partida *partida, bool painel_aberto) {
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

static void hud_desenhar_alerta(const Partida *partida, float tempo) {
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

static void hud_desenhar_dica(const Partida *partida) {
    const Tarefa *proxima = partida_proxima_obrigatoria(partida);
    if (!proxima || !proxima->dica || !proxima->dica[0]) return;
    float largura = render_text_width(proxima->dica, 14) + 30;
    Rectangle caixa = {(VIRTUAL_WIDTH - largura) / 2, HUD_Y_RODAPE - 38, largura, 28};
    DrawRectangleRec(caixa, Fade(COLOR_BAR, 0.85f));
    DrawRectangleLinesEx(caixa, 1, Fade(COLOR_SIGNAL, 0.45f));
    render_text(proxima->dica, caixa.x + 15, caixa.y + 6, 14, Fade(COLOR_CREAM, 0.9f));
}

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

static void hud_desenhar_tarefas(const Partida *partida) {
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

static void hud_desenhar_balao(Vector2 posicao, const char *rotulo, float tempo) {
    float largura = render_text_width(rotulo, 14) + 46;
    float balanco = sinf(tempo * 3.0f) * 2.0f;
    Rectangle balao = {posicao.x - largura / 2, posicao.y - 100 + balanco, largura, 30};
    DrawRectangleRec(balao, Fade(COLOR_BAR, 0.9f));
    DrawRectangleLinesEx(balao, 1, Fade(COLOR_AMBER, 0.7f));
    ui_tecla("E", balao.x + 6, balao.y + 4);
    render_text(rotulo, balao.x + 36, balao.y + 7, 14, COLOR_CREAM);
}

static void hud_desenhar_aviso(const char *texto, float alpha, bool em_painel) {
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

/* ---------- painel do receptor: novidades, fila de sinais e mensagens ---------- */
#define RECEPTOR_LISTA_LARGURA 280
#define RECEPTOR_ITEM_ALTURA 44

static const char *estado_da_entrada(const EntradaCaixa *entrada, const Mensagem *mensagem) {
    if (entrada->transmitida) return "transmitida";
    if (entrada->descartada) return "riscada";
    if (entrada->contradicao) return "ALGO NÃO FECHA";
    if (entrada->confirmada) return mensagem->veracidade == VERACIDADE_FALSA ? "FALSA (confirmado)" : "verdadeira (confirmado)";
    if (entrada->aceita) return "no roteiro";
    return "nova";
}

static Color cor_do_estado(const EntradaCaixa *entrada, const Mensagem *mensagem) {
    if (entrada->contradicao || (entrada->confirmada && mensagem->veracidade == VERACIDADE_FALSA)) return COLOR_ALERT;
    if (entrada->aceita || entrada->transmitida) return COLOR_SIGNAL;
    return COLOR_MUTED;
}

static void desenhar_abas(GameState *game, Rectangle area) {
    const char *mensagens = TextFormat("MENSAGENS (%d)", game->partida.total_caixa);
    if (ui_item_lista((Rectangle){area.x, area.y - 6, 180, 34}, "NOVIDADES", NULL, game->receptor_em_novidades, COLOR_MUTED))
        game->receptor_em_novidades = true;
    if (ui_item_lista((Rectangle){area.x + 190, area.y - 6, 200, 34}, mensagens, NULL, !game->receptor_em_novidades, COLOR_MUTED))
        game->receptor_em_novidades = false;
}

static void desenhar_novidades(GameState *game, Rectangle area) {
    partida_ler_novidades(&game->partida);
    float y = area.y + 48;
    int total;
    const Novidade *novidades = campanha_novidades(&total);
    for (int i = 0; i < total; i++) {
        if (!partida_novidade_visivel(&game->partida, &novidades[i])) continue;
        render_text(novidades[i].titulo, area.x + 10, y, 15, COLOR_AMBER);
        y += 24 + render_text_wrapped(novidades[i].texto, (Rectangle){area.x + 10, y + 24, area.width - 20, 0}, 18, COLOR_CREAM);
        y += 22;
    }
    if (game->partida.fase == FASE_PREPARACAO)
        render_text("Depois de ler, escolha na bancada como se preparar e ligue a rádio no transmissor.", area.x + 10, area.y + area.height - 30, 15, COLOR_SIGNAL);
}

static float desenhar_fila(GameState *game, Rectangle area) {
    Partida *partida = &game->partida;
    Rectangle caixa = {area.x, area.y + 40, RECEPTOR_LISTA_LARGURA, 86};
    DrawRectangleRounded(caixa, 0.1f, 6, COLOR_BAR);
    render_text("SINAIS EM ESPERA", caixa.x + 12, caixa.y + 10, 12, COLOR_MUTED);
    render_text(TextFormat("%d na escuta", partida->fila.tamanho), caixa.x + 12, caixa.y + 26, 16, partida->fila.tamanho ? COLOR_SIGNAL : COLOR_MUTED);
    if (ui_botao_custo((Rectangle){caixa.x + 10, caixa.y + 48, caixa.width - 20, 30}, "Sintonizar o próximo", 0, partida->fila.tamanho > 0)) {
        if (partida_receber_mensagem(partida)) game->caixa_selecionada = partida->total_caixa - 1;
    }
    return caixa.y + caixa.height + 10;
}

static void desenhar_caixa(GameState *game, Rectangle area, float y) {
    for (int i = 0; i < game->partida.total_caixa && y + RECEPTOR_ITEM_ALTURA < area.y + area.height; i++) {
        const EntradaCaixa *entrada = &game->partida.caixa[i];
        const Mensagem *mensagem = campanha_mensagem(entrada->id_mensagem);
        Rectangle item = {area.x, y, RECEPTOR_LISTA_LARGURA, RECEPTOR_ITEM_ALTURA};
        if (ui_item_lista(item, mensagem->remetente, estado_da_entrada(entrada, mensagem), i == game->caixa_selecionada,
                          cor_do_estado(entrada, mensagem)))
            game->caixa_selecionada = i;
        y += RECEPTOR_ITEM_ALTURA + 4;
    }
}

static float desenhar_descobertas(const EntradaCaixa *entrada, const Mensagem *mensagem, Rectangle coluna, float y) {
    if (entrada->confirmada) {
        Color cor = mensagem->veracidade == VERACIDADE_FALSA ? COLOR_ALERT : COLOR_SIGNAL;
        render_text("RESPOSTA DA REDE", coluna.x, y, 12, cor);
        y += 18 + render_text_wrapped(mensagem->confirmacao, (Rectangle){coluna.x, y + 18, coluna.width, 0}, 15, cor) + 8;
    }
    if (entrada->contradicao) {
        render_text("ALGO NÃO FECHA", coluna.x, y, 12, COLOR_ALERT);
        y += 18 + render_text_wrapped(mensagem->contradicao, (Rectangle){coluna.x, y + 18, coluna.width, 0}, 15, COLOR_ALERT) + 8;
    }
    return y;
}

static float desenhar_previa(const Mensagem *mensagem, Rectangle coluna, float y) {
    if (!mensagem->transmissivel) return y;
    const Transmissao *transmissao = &mensagem->transmissao;
    Rectangle caixa = {coluna.x, y, coluna.width, 50};
    DrawRectangleRounded(caixa, 0.15f, 6, COLOR_BAR);
    render_text(TextFormat("No roteiro vira: %s", transmissao->titulo), caixa.x + 12, caixa.y + 7, 14, COLOR_CREAM);
    char limite[6];
    partida_formatar_hora(transmissao->prazo, limite);
    const char *requer = transmissao->requer[0] ? TextFormat(" · só depois da senha %s", transmissao->requer) : "";
    render_text(TextFormat("%d min no ar · vale até %s%s", transmissao->custo * MINUTOS_POR_ACAO, limite, requer), caixa.x + 12, caixa.y + 28, 13, COLOR_MUTED);
    return y + 60;
}

static float desenhar_respostas(GameState *game, const EntradaCaixa *entrada, const Mensagem *mensagem, Rectangle coluna, float y) {
    if (mensagem->total_respostas == 0) return y;
    if (entrada->resposta >= 0) {
        render_text(TextFormat("Sua resposta: %s", mensagem->respostas[entrada->resposta].rotulo), coluna.x, y, 14, COLOR_SIGNAL);
        return y + 26;
    }
    for (int r = 0; r < mensagem->total_respostas; r++) {
        const RespostaMensagem *resposta = &mensagem->respostas[r];
        bool pode = resposta->custo == 0 || game->partida.fase == FASE_OPERACAO;
        if (ui_botao_custo((Rectangle){coluna.x, y, coluna.width, 34}, resposta->rotulo, resposta->custo, pode))
            partida_responder(&game->partida, game->caixa_selecionada, r);
        y += 40;
    }
    return y;
}

static void desenhar_botoes(GameState *game, const EntradaCaixa *entrada, const Mensagem *mensagem, Rectangle coluna) {
    Partida *partida = &game->partida;
    float y = coluna.y + coluna.height - 40, largura = (coluna.width - 20) / 3;
    bool aberta = !entrada->transmitida && !entrada->descartada;
    bool pode_confirmar = !entrada->confirmada && partida->fase == FASE_OPERACAO;
    if (ui_botao_custo((Rectangle){coluna.x, y, largura, 38}, "Confirmar", partida_custo_confirmacao(partida), pode_confirmar))
        partida_confirmar(partida, game->caixa_selecionada);
    bool pode_aceitar = aberta && mensagem->transmissivel && !entrada->aceita;
    if (ui_botao_custo((Rectangle){coluna.x + largura + 10, y, largura, 38}, "Pôr no roteiro", 0, pode_aceitar))
        partida_aceitar(partida, game->caixa_selecionada);
    if (ui_botao_custo((Rectangle){coluna.x + 2 * (largura + 10), y, largura, 38}, "Riscar", 0, aberta))
        partida_descartar(partida, game->caixa_selecionada);
}

static void desenhar_mensagem(GameState *game, Rectangle coluna) {
    int indice = game->caixa_selecionada;
    const EntradaCaixa *entrada = indice >= 0 && indice < game->partida.total_caixa ? &game->partida.caixa[indice] : NULL;
    if (!entrada) {
        render_text("Sintonize um sinal ou escolha uma mensagem anotada.", coluna.x, coluna.y + 10, 17, COLOR_MUTED);
        return;
    }
    const Mensagem *mensagem = campanha_mensagem(entrada->id_mensagem);
    render_text(mensagem->remetente, coluna.x, coluna.y, 22, COLOR_AMBER);
    render_text(estado_da_entrada(entrada, mensagem), coluna.x + render_text_width(mensagem->remetente, 22) + 16, coluna.y + 6, 13, cor_do_estado(entrada, mensagem));
    float y = coluna.y + 36 + render_text_wrapped(mensagem->texto, (Rectangle){coluna.x, coluna.y + 36, coluna.width, 0}, 18, COLOR_CREAM) + 10;
    y = desenhar_descobertas(entrada, mensagem, coluna, y);
    y = desenhar_previa(mensagem, coluna, y);
    desenhar_respostas(game, entrada, mensagem, coluna, y);
    desenhar_botoes(game, entrada, mensagem, coluna);
}

static void painel_receptor(GameState *game, Rectangle area) {
    desenhar_abas(game, area);
    if (game->receptor_em_novidades) {
        desenhar_novidades(game, area);
        return;
    }
    float y = desenhar_fila(game, area);
    desenhar_caixa(game, area, y);
    Rectangle coluna = {area.x + RECEPTOR_LISTA_LARGURA + 24, area.y + 44, area.width - RECEPTOR_LISTA_LARGURA - 24, area.height - 44};
    desenhar_mensagem(game, coluna);
}

/* ---------- painel da bancada: preparativo, documentos e comparação ---------- */
#define BANCADA_LISTA_LARGURA 260
#define BANCADA_ITEM_ALTURA 38

static float desenhar_preparativos(GameState *game, Rectangle area) {
    Partida *partida = &game->partida;
    if (partida->fase != FASE_PREPARACAO) return area.y;
    render_text("COMO SE PREPARAR PARA ESTA NOITE (escolha um)", area.x, area.y, 14, COLOR_MUTED);
    float largura = (area.width - 3 * 12) / 4;
    for (int p = PREPARO_BATERIA; p < PREPARO_TOTAL; p++) {
        const DescricaoPreparativo *descricao = campanha_preparativo((Preparativo)p);
        Rectangle cartao = {area.x + (p - 1) * (largura + 12), area.y + 22, largura, 80};
        bool escolhido = partida->preparativo == (Preparativo)p;
        DrawRectangleRounded(cartao, 0.12f, 6, escolhido ? COLOR_CARD_SELECTED : ui_mouse_sobre(cartao) ? (Color){42, 48, 62, 255} : COLOR_CARD);
        if (escolhido) DrawRectangleRoundedLinesEx(cartao, 0.12f, 6, 2, COLOR_AMBER);
        render_text(descricao->nome, cartao.x + 12, cartao.y + 8, 15, COLOR_CREAM);
        render_text_wrapped(descricao->descricao, (Rectangle){cartao.x + 12, cartao.y + 30, cartao.width - 24, 0}, 13, COLOR_MUTED);
        if (ui_clicou(cartao)) partida_escolher_preparativo(partida, (Preparativo)p);
    }
    return area.y + 112;
}

static void desenhar_lista_documentos(GameState *game, Rectangle area, float y) {
    int total;
    const Documento *documentos = campanha_documentos(&total);
    render_text("DOCUMENTOS", area.x, y, 12, COLOR_MUTED);
    y += 20;
    for (int i = 0; i < total; i++) {
        const Documento *documento = &documentos[i];
        if (!partida_documento_na_bancada(&game->partida, documento)) continue;
        bool liberado = partida_documento_liberado(&game->partida, documento);
        Rectangle item = {area.x, y, BANCADA_LISTA_LARGURA, BANCADA_ITEM_ALTURA};
        const char *detalhe = liberado ? (documento->noite == game->partida.noite ? "novo nesta noite" : "") : "ainda não obtido";
        if (ui_item_lista(item, liberado ? documento->titulo : "???", detalhe, game->documento_selecionado == documento->id,
                          liberado ? COLOR_SIGNAL : COLOR_AMBER))
            game->documento_selecionado = documento->id;
        y += BANCADA_ITEM_ALTURA + 4;
    }
}

static void desenhar_comparacao(GameState *game, Rectangle coluna, int id_documento) {
    Partida *partida = &game->partida;
    float y = coluna.y + coluna.height - 46;
    render_text("COMPARAR ESTE DOCUMENTO COM A MENSAGEM DE:", coluna.x, y - 22, 12, COLOR_MUTED);
    if (partida->total_caixa == 0) {
        render_text("Nenhuma mensagem anotada ainda.", coluna.x, y + 8, 15, COLOR_MUTED);
        return;
    }
    if (game->mensagem_comparada >= partida->total_caixa || game->mensagem_comparada < 0) game->mensagem_comparada = 0;
    if (ui_botao((Rectangle){coluna.x, y, 38, 38}, "<", true))
        game->mensagem_comparada = (game->mensagem_comparada + partida->total_caixa - 1) % partida->total_caixa;
    const Mensagem *mensagem = partida_mensagem_da_caixa(partida, game->mensagem_comparada);
    Rectangle nome = {coluna.x + 46, y, coluna.width - 46 - 46 - 170, 38};
    DrawRectangleRounded(nome, 0.2f, 6, COLOR_BAR);
    render_text(TextFormat("%d. %s", game->mensagem_comparada + 1, mensagem->remetente), nome.x + 12, nome.y + 10, 15, COLOR_CREAM);
    if (ui_botao((Rectangle){nome.x + nome.width + 8, y, 38, 38}, ">", true))
        game->mensagem_comparada = (game->mensagem_comparada + 1) % partida->total_caixa;
    if (ui_botao_custo((Rectangle){coluna.x + coluna.width - 160, y, 160, 38}, "Comparar", 0, true))
        partida_comparar(partida, game->mensagem_comparada, id_documento);
}

static void desenhar_documento(GameState *game, Rectangle coluna) {
    const Documento *documento = campanha_documento(game->documento_selecionado);
    if (!documento || !partida_documento_na_bancada(&game->partida, documento)) {
        render_text("Selecione um documento.", coluna.x, coluna.y + 10, 17, COLOR_MUTED);
        return;
    }
    if (!partida_documento_liberado(&game->partida, documento)) {
        render_text("Documento ainda não obtido", coluna.x, coluna.y, 20, COLOR_AMBER);
        render_text_wrapped("Conseguir este documento leva tempo, mas ele pode ser a peça que falta para desmascarar alguém.",
                            (Rectangle){coluna.x, coluna.y + 34, coluna.width, 0}, 16, COLOR_CREAM);
        if (ui_botao_custo((Rectangle){coluna.x, coluna.y + 96, 260, 40}, "Investigar", 1, game->partida.fase == FASE_OPERACAO))
            partida_investigar(&game->partida, documento->id);
        return;
    }
    Rectangle papel = {coluna.x, coluna.y, coluna.width, coluna.height - 82};
    DrawRectangleRounded(papel, 0.04f, 6, (Color){222, 212, 186, 255});
    render_text(documento->titulo, papel.x + 18, papel.y + 14, 20, (Color){60, 40, 30, 255});
    render_text_wrapped(documento->texto, (Rectangle){papel.x + 18, papel.y + 46, papel.width - 36, 0}, 17, (Color){40, 32, 28, 255});
    desenhar_comparacao(game, coluna, documento->id);
}

static void desenhar_esconderijo(GameState *game, Rectangle area) {
    Partida *partida = &game->partida;
    if (partida->inspecao == EMERGENCIA_INATIVA || partida->fase != FASE_OPERACAO) return;
    Rectangle botao = {area.x, area.y + area.height - 40, BANCADA_LISTA_LARGURA, 40};
    if (partida->provas_escondidas) {
        render_text("Papéis escondidos sob o piso.", botao.x, botao.y + 12, 14, COLOR_SIGNAL);
        return;
    }
    if (ui_botao_custo(botao, "Esconder os papéis", partida_custo_esconder(partida), partida->fase == FASE_OPERACAO))
        partida_esconder_provas(partida);
}

static void painel_bancada(GameState *game, Rectangle area) {
    float y = desenhar_preparativos(game, area);
    desenhar_lista_documentos(game, area, y);
    Rectangle coluna = {area.x + BANCADA_LISTA_LARGURA + 24, y + 20, area.width - BANCADA_LISTA_LARGURA - 24, area.y + area.height - y - 20};
    desenhar_documento(game, coluna);
    desenhar_esconderijo(game, area);
}

/* ---------- painel do transmissor: o roteiro da noite ---------- */
#define MAX_LINHAS 6
#define LINHA_X 240
#define LINHA_LARGURA 570
#define LINHA_ALTURA 50
#define LINHA_INTERVALO 16
#define LINHAS_TOPO 192
#define COLUNA_X 836
#define COLUNA_LARGURA 270

typedef struct {
    GameState *game;
    int acao_prevista;
    char codigos_antes[16][TEXTO_CODIGO];
    int total_codigos;
} ContextoRoteiro;

static Rectangle retangulo_linha(int posicao) {
    return (Rectangle){LINHA_X, LINHAS_TOPO + posicao * (LINHA_ALTURA + LINHA_INTERVALO), LINHA_LARGURA, LINHA_ALTURA};
}

static bool senha_disponivel(const ContextoRoteiro *contexto, const char *codigo) {
    if (partida_codigo_no_ar(&contexto->game->partida, codigo)) return true;
    for (int i = 0; i < contexto->total_codigos; i++) {
        if (strcmp(contexto->codigos_antes[i], codigo) == 0) return true;
    }
    return false;
}

static float etiqueta_a_esquerda(float x, float y, const char *texto, Color cor) {
    x -= render_text_width(texto, 12) + 22;
    ui_etiqueta(x, y, texto, cor);
    return x;
}

static void desenhar_avisos_da_linha(Rectangle linha, const Transmissao *dados, const ContextoRoteiro *contexto) {
    float x = linha.x + linha.width - 6;
    if (dados->urgente) x = etiqueta_a_esquerda(x, linha.y + 6, "URGENTE", COLOR_ALERT);
    if (contexto->acao_prevista > dados->prazo) x = etiqueta_a_esquerda(x, linha.y + 6, "VAI ATRASAR", COLOR_ALERT);
    if (dados->requer[0] && !senha_disponivel(contexto, dados->requer))
        etiqueta_a_esquerda(x, linha.y + 6, TextFormat("FALTA A SENHA %s", dados->requer), COLOR_AMBER);
}

static void desenhar_fio(Rectangle linha) {
    float x = linha.x + 26, topo = linha.y + linha.height, base = topo + LINHA_INTERVALO;
    DrawLineEx((Vector2){x, topo}, (Vector2){x, base}, 2, Fade(COLOR_EDGE, 0.6f));
}

static void detalhe_da_linha(const Transmissao *dados, char *saida, size_t tamanho) {
    char limite[6];
    partida_formatar_hora(dados->prazo, limite);
    if (dados->requer[0]) snprintf(saida, tamanho, "%d min · vale até %s · só depois da senha %s", dados->custo * MINUTOS_POR_ACAO, limite, dados->requer);
    else snprintf(saida, tamanho, "%d min · vale até %s", dados->custo * MINUTOS_POR_ACAO, limite);
}

/* Visitante chamado por exibirProgramacao para cada nó da lista. Também simula a noite
   para avisar se a transmissão vai atrasar ou se falta a senha antes dela. */
static void desenhar_no(const NoTransmissao *no, int posicao, void *dados_contexto) {
    ContextoRoteiro *contexto = dados_contexto;
    const Transmissao *dados = &no->dados;
    contexto->acao_prevista += dados->custo;
    if (posicao < MAX_LINHAS) {
        Rectangle linha = retangulo_linha(posicao);
        if (ui_item_lista(linha, "", NULL, posicao == contexto->game->programacao_selecionada, COLOR_MUTED))
            contexto->game->programacao_selecionada = posicao;
        DrawCircle((int)linha.x + 26, (int)(linha.y + linha.height / 2), 14, COLOR_BAR);
        render_text_centered(TextFormat("%d", posicao + 1), linha.x + 26, linha.y + linha.height / 2 - 10, 19, COLOR_AMBER);
        render_text(dados->titulo, linha.x + 52, linha.y + 6, 16, COLOR_CREAM);
        char detalhe[TEXTO_TITULO];
        detalhe_da_linha(dados, detalhe, sizeof(detalhe));
        render_text(detalhe, linha.x + 52, linha.y + 30, 12, COLOR_MUTED);
        desenhar_avisos_da_linha(linha, dados, contexto);
        if (no->proximo && posicao + 1 < MAX_LINHAS) desenhar_fio(linha);
    }
    if (contexto->total_codigos < 16) snprintf(contexto->codigos_antes[contexto->total_codigos++], TEXTO_CODIGO, "%s", dados->codigo);
}

static void desenhar_roteiro(GameState *game, Rectangle area) {
    Programacao *programacao = &game->partida.programacao;
    render_text("O primeiro da lista é o próximo a ir ao ar. A ordem decide quem ouve o quê, e a tempo.", area.x + 6, area.y, 14, COLOR_MUTED);
    if (!programacao->inicio) {
        render_text("Roteiro vazio. Escolha no receptor o que deve ir ao ar.", LINHA_X, LINHAS_TOPO + 20, 17, COLOR_MUTED);
        return;
    }
    ContextoRoteiro contexto = {.game = game, .acao_prevista = game->partida.relogio};
    exibirProgramacao(programacao, desenhar_no, &contexto);
    Rectangle primeira = retangulo_linha(0);
    render_text("A SEGUIR", 168, primeira.y + 17, 13, COLOR_ALERT);
    if (programacao->tamanho > MAX_LINHAS) render_text(TextFormat("+%d mais abaixo", programacao->tamanho - MAX_LINHAS), LINHA_X, 586, 13, COLOR_MUTED);
}

static void corrigir_selecao(GameState *game) {
    int tamanho = game->partida.programacao.tamanho;
    if (game->programacao_selecionada >= tamanho) game->programacao_selecionada = tamanho - 1;
    if (game->programacao_selecionada < 0 && tamanho > 0) game->programacao_selecionada = 0;
}

static NoTransmissao *no_selecionado(GameState *game) {
    return buscarTransmissaoNaPosicao(&game->partida.programacao, game->programacao_selecionada);
}

static void mover_selecionado(GameState *game, int direcao) {
    if (partida_mover_transmissao(&game->partida, no_selecionado(game), direcao)) game->programacao_selecionada += direcao;
}

static void tratar_teclas(GameState *game) {
    bool shift = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
    int direcao = (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) - (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W));
    if (direcao && shift) mover_selecionado(game, direcao);
    else if (direcao) game->programacao_selecionada += direcao;
    if (IsKeyPressed(KEY_P)) partida_priorizar(&game->partida);
    if (IsKeyPressed(KEY_R)) partida_remover_transmissao(&game->partida, no_selecionado(game));
    if (IsKeyPressed(KEY_ENTER) && game->partida.fase == FASE_OPERACAO) partida_transmitir_proxima(&game->partida);
    corrigir_selecao(game);
}

static float desenhar_estado(const Partida *partida, float y) {
    char agora[6], fim[6];
    partida_formatar_hora(partida->relogio, agora);
    partida_formatar_hora(partida->relogio + partida->pontos_acao, fim);
    render_text(agora, COLUNA_X, y - 4, 26, COLOR_CREAM);
    render_text(TextFormat("bateria até %s", fim), COLUNA_X + 90, y + 4, 14, COLOR_AMBER);
    ui_medidor((Rectangle){COLUNA_X, y + 30, COLUNA_LARGURA, 34}, "SINAL EXPOSTO", partida->rastreamento,
               partida->rastreamento >= 70 ? COLOR_ALERT : COLOR_SIGNAL);
    return y + 78;
}

static float desenhar_botoes_roteiro(GameState *game, float y) {
    NoTransmissao *selecionado = no_selecionado(game);
    if (ui_botao((Rectangle){COLUNA_X, y, 130, 38}, "Subir", selecionado && selecionado->anterior)) mover_selecionado(game, -1);
    if (ui_botao((Rectangle){COLUNA_X + 140, y, 130, 38}, "Descer", selecionado && selecionado->proximo)) mover_selecionado(game, 1);
    if (ui_botao_custo((Rectangle){COLUNA_X, y + 46, COLUNA_LARGURA, 38}, "Priorizar urgentes", 0, game->partida.programacao.tamanho > 1))
        partida_priorizar(&game->partida);
    if (ui_botao_custo((Rectangle){COLUNA_X, y + 92, COLUNA_LARGURA, 38}, "Tirar do roteiro", 0, selecionado != NULL))
        partida_remover_transmissao(&game->partida, selecionado);
    corrigir_selecao(game);
    return y + 142;
}

static float desenhar_acao_principal(GameState *game, float y) {
    Partida *partida = &game->partida;
    if (partida->fase == FASE_PREPARACAO) {
        bool pronto = partida->preparativo != PREPARO_NENHUM;
        if (ui_botao_principal((Rectangle){COLUNA_X, y, COLUNA_LARGURA, 56}, "LIGAR A RÁDIO", -1, pronto)) partida_iniciar_operacao(partida);
        render_text(pronto ? "Depois disso, cada decisão consome bateria." : "Antes, escolha como se preparar na bancada.", COLUNA_X, y + 64, 12, COLOR_MUTED);
        return y + 90;
    }
    NoTransmissao *primeiro = partida->programacao.inicio;
    int custo = primeiro ? primeiro->dados.custo : 0;
    bool pode = primeiro && partida->fase == FASE_OPERACAO && partida->equipamentos_ligados && partida->pontos_acao >= custo;
    if (ui_botao_principal((Rectangle){COLUNA_X, y, COLUNA_LARGURA, 56}, "TRANSMITIR", custo, pode)) partida_transmitir_proxima(partida);
    render_text(primeiro ? TextFormat("Vai ao ar: %s  (ENTER)", primeiro->dados.titulo) : "Nada no roteiro.", COLUNA_X, y + 64, 12, COLOR_MUTED);
    return y + 90;
}

static void desenhar_equipamentos(GameState *game, float y) {
    Partida *partida = &game->partida;
    if (partida->inspecao == EMERGENCIA_INATIVA) return;
    bool ligados = partida->equipamentos_ligados;
    if (ui_botao_custo((Rectangle){COLUNA_X, y, COLUNA_LARGURA, 38}, ligados ? "Desligar tudo" : "Religar os equipamentos",
                       ligados ? 1 : 0, partida->fase == FASE_OPERACAO))
        partida_alternar_equipamentos(partida);
}

static void painel_transmissor(GameState *game, Rectangle area) {
    tratar_teclas(game);
    desenhar_roteiro(game, area);
    float y = desenhar_estado(&game->partida, area.y + 34);
    y = desenhar_botoes_roteiro(game, y);
    y = desenhar_acao_principal(game, y);
    desenhar_equipamentos(game, y);
}

/* ---------- painéis de frequência, janela, porta, colchão e objetos ---------- */
static void desenhar_onda(Rectangle area, float tempo, float amplitude) {
    DrawRectangleRounded(area, 0.04f, 6, (Color){8, 20, 16, 255});
    Vector2 anterior = {area.x, area.y + area.height / 2};
    for (float x = 2; x <= area.width; x += 2) {
        float y = area.y + area.height / 2 + sinf(x * 0.08f + tempo * 5.0f) * amplitude * sinf(x * 0.01f + tempo);
        Vector2 ponto = {area.x + x, y};
        DrawLineEx(anterior, ponto, 2, COLOR_SIGNAL);
        anterior = ponto;
    }
    ui_scanlines(area);
}

static void painel_frequencia(GameState *game, Rectangle area) {
    Partida *partida = &game->partida;
    float amplitude = 20.0f + partida->rastreamento * 0.9f;
    desenhar_onda((Rectangle){area.x, area.y, 560, 260}, game->clock, amplitude);
    Rectangle coluna = {area.x + 590, area.y, area.width - 590, area.height};
    ui_medidor((Rectangle){coluna.x, coluna.y, coluna.width, 44}, "SINAL EXPOSTO", partida->rastreamento,
               partida->rastreamento >= 70 ? COLOR_ALERT : COLOR_SIGNAL);
    render_text_wrapped("Cada transmissão deixa um rastro no ar. Se o sinal ficar totalmente exposto, um carro de escuta acha a "
                        "rádio e a noite acaba. Trocar de frequência embaralha o rastro.",
                        (Rectangle){coluna.x, coluna.y + 64, coluna.width, 0}, 15, COLOR_CREAM);
    if (ui_botao_principal((Rectangle){coluna.x, coluna.y + 200, coluna.width, 54}, "Trocar de frequência", 1,
                           partida->fase == FASE_OPERACAO && partida->rastreamento > 0))
        partida_trocar_frequencia(partida);
    if (partida->fase == FASE_PREPARACAO) render_text("Disponível depois de ligar a rádio.", coluna.x, coluna.y + 264, 13, COLOR_MUTED);
}

static void painel_janela(GameState *game, Rectangle area) {
    Partida *partida = &game->partida;
    render_text(partida->cortina_fechada ? "A cortina está fechada." : "A cortina está aberta.", area.x, area.y, 22, COLOR_AMBER);
    char hora[6];
    partida_formatar_hora(partida->holofote_chegada, hora);
    const char *situacao = partida->holofote == EMERGENCIA_AVISADA
                               ? TextFormat("A chuva escorre pelo vidro. Às %s o holofote da torre vai varrer esta janela: quem estiver "
                                            "na luz, com a cortina aberta, será visto.", hora)
                           : partida->holofote == EMERGENCIA_RESOLVIDA ? "O holofote já passou por aqui esta noite. Por enquanto, só chuva."
                           : partida->holofote == EMERGENCIA_AGUARDANDO ? "A torre está escura, mas o holofote está lá. Ele vai virar para cá."
                                                                        : "Só a chuva e a rua vazia. Nenhum holofote esta noite.";
    render_text_wrapped(situacao, (Rectangle){area.x, area.y + 44, area.width, 0}, 18, COLOR_CREAM);
    render_text_wrapped("Puxar a cortina leva tempo. A outra saída é simplesmente não estar na luz quando ela passar.",
                        (Rectangle){area.x, area.y + 130, area.width, 0}, 15, COLOR_MUTED);
    const char *rotulo = partida->cortina_fechada ? "Abrir a cortina" : "Fechar a cortina";
    if (ui_botao_principal((Rectangle){area.x, area.y + 190, 340, 54}, rotulo, 1, partida->fase == FASE_OPERACAO))
        partida_alternar_cortina(partida);
}

static void desenhar_checagem(float x, float y, const char *texto, bool ok) {
    render_text(ok ? "OK" : "!!", x, y, 16, ok ? COLOR_SIGNAL : COLOR_ALERT);
    render_text(texto, x + 40, y, 17, ok ? COLOR_CREAM : COLOR_ALERT);
}

static void painel_porta(GameState *game, Rectangle area) {
    Partida *partida = &game->partida;
    int faltam = partida_acoes_ate_inspecao(partida);
    if (faltam < 0) {
        const char *texto = partida->inspecao_superada ? "Os passos se afastaram escada abaixo. O corredor voltou ao silêncio."
                                                       : "Pelo olho mágico, só o corredor vazio e a lâmpada piscando.";
        render_text_wrapped(texto, (Rectangle){area.x, area.y, area.width, 0}, 20, COLOR_CREAM);
        return;
    }
    char hora[6];
    partida_formatar_hora(partida->inspecao_chegada, hora);
    render_text(TextFormat("Botas na escada. Eles batem à porta às %s.", hora), area.x, area.y, 22, COLOR_ALERT);
    const char *suspeita = partida->suspeita < 25 ? "baixa" : partida->suspeita < 50 ? "média" : "alta";
    desenhar_checagem(area.x, area.y + 54, "Papéis escondidos (bancada)", partida->provas_escondidas);
    desenhar_checagem(area.x, area.y + 86, "Equipamentos desligados (transmissor)", !partida->equipamentos_ligados);
    desenhar_checagem(area.x, area.y + 118, TextFormat("Desconfiança sobre você: %s", suspeita), partida->suspeita < 40);
    render_text_wrapped("Se atender agora, a revista acontece já. Se esperar, eles entram na hora marcada de qualquer jeito. "
                        "Papel à vista é o fim; um transmissor quente e uma ficha suja pioram tudo.",
                        (Rectangle){area.x, area.y + 170, area.width, 0}, 15, COLOR_MUTED);
    if (ui_botao_principal((Rectangle){area.x, area.y + 250, 340, 54}, "Atender a porta", 0, true)) partida_responder_porta(partida);
}

static void painel_colchao(GameState *game, Rectangle area) {
    Partida *partida = &game->partida;
    if (partida->fase != FASE_OPERACAO) {
        render_text_wrapped("Ainda não. A noite nem começou: prepare-se e ligue a rádio no transmissor.",
                            (Rectangle){area.x, area.y, area.width, 0}, 20, COLOR_CREAM);
        return;
    }
    char fim[6];
    partida_formatar_hora(partida->relogio + partida->pontos_acao, fim);
    render_text("Deitar agora encerra a noite.", area.x, area.y, 22, COLOR_CREAM);
    render_text(TextFormat("A bateria ainda aguentaria até %s. Esse tempo se perde.", fim), area.x, area.y + 34, 16, COLOR_MUTED);
    float y = area.y + 76;
    int total;
    const Tarefa *tarefas = campanha_tarefas(&total);
    render_text("AINDA FALTA:", area.x, y, 13, COLOR_AMBER);
    y += 24;
    bool alguma = false;
    for (int i = 0; i < total; i++) {
        if (!tarefas[i].obrigatoria || !partida_tarefa_visivel(partida, &tarefas[i]) || partida_tarefa_concluida(partida, &tarefas[i])) continue;
        render_text(TextFormat("- %s", tarefas[i].texto), area.x + 10, y, 16, COLOR_ALERT);
        y += 26;
        alguma = true;
    }
    if (!alguma) render_text("Nada essencial. Você fez o que precisava.", area.x + 10, y, 16, COLOR_SIGNAL);
    if (ui_botao_principal((Rectangle){area.x, area.y + area.height - 70, 360, 54}, "Apagar a luz e dormir", -1, true))
        partida_dormir(partida);
}

static void painel_objeto(GameState *game, Rectangle area) {
    const TextoObjeto *objeto = campanha_objeto(game->objeto_aberto);
    render_text_wrapped(objeto->texto, (Rectangle){area.x, area.y + 10, area.width, 0}, 20, COLOR_CREAM);
    render_text("Olhar não custa tempo.", area.x, area.y + area.height - 24, 14, COLOR_MUTED);
}

/* ---------- a noite: atualização e desenho ---------- */
#define DURACAO_AVISO 7.0f
#define DURACAO_CLARAO 1.8f
#define CICLO_RELAMPAGO 43.0f
#define ATRASO_TROVAO 1.6f
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

static float fase_do_relampago(float tempo) { return fmodf(tempo + 7.0f, CICLO_RELAMPAGO); }

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

static float tensao_da_noite(const Partida *partida) {
    if (partida->fase != FASE_OPERACAO) return 0.0f;
    float tensao = 0.15f;
    int holofote = partida_acoes_ate_holofote(partida), inspecao = partida_acoes_ate_inspecao(partida);
    if (holofote >= 0) tensao = fmaxf(tensao, 1.0f - holofote * 0.15f);
    if (inspecao >= 0) tensao = fmaxf(tensao, 1.0f - inspecao * 0.12f);
    if (partida->rastreamento >= 70) tensao = fmaxf(tensao, 0.6f);
    return tensao;
}

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

/* Mostra na tela o texto e o som que a última ação da partida deixou. */
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
    pixel_luz((float)px(game->player.position.x), (float)(player_feet_pixel(&game->player) - 10), 34, (Color){255, 214, 170, 255}, 0.4f);
    pixel_luz_terminar();
    pixel_mundo_comecar(BLACK);
    cabin_draw(game->clock, &cena, player_feet_pixel(&game->player), desenhar_jogador, &game->player);
    pixel_aplicar_luz();
    cabin_draw_emissive(game->clock, destacado, &cena);
    pixel_vinheta(1.0f);
    pixel_mundo_terminar();
}

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
