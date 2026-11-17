/* Receptor: novidades da noite, fila de entrada (FIFO) e caixa de mensagens recebidas. */
#include "paineis.h"
#include "render.h"
#include "widgets.h"
#include <stddef.h>

#define LISTA_LARGURA 280
#define ITEM_ALTURA 44

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
    Rectangle caixa = {area.x, area.y + 40, LISTA_LARGURA, 86};
    DrawRectangleRounded(caixa, 0.1f, 6, COLOR_BAR);
    render_text("SINAIS EM ESPERA", caixa.x + 12, caixa.y + 10, 12, COLOR_MUTED);
    render_text(TextFormat("%d na escuta", partida->fila.tamanho), caixa.x + 12, caixa.y + 26, 16, partida->fila.tamanho ? COLOR_SIGNAL : COLOR_MUTED);
    if (ui_botao_custo((Rectangle){caixa.x + 10, caixa.y + 48, caixa.width - 20, 30}, "Sintonizar o próximo", 0, partida->fila.tamanho > 0)) {
        if (partida_receber_mensagem(partida)) game->caixa_selecionada = partida->total_caixa - 1;
    }
    return caixa.y + caixa.height + 10;
}

static void desenhar_caixa(GameState *game, Rectangle area, float y) {
    for (int i = 0; i < game->partida.total_caixa && y + ITEM_ALTURA < area.y + area.height; i++) {
        const EntradaCaixa *entrada = &game->partida.caixa[i];
        const Mensagem *mensagem = campanha_mensagem(entrada->id_mensagem);
        Rectangle item = {area.x, y, LISTA_LARGURA, ITEM_ALTURA};
        if (ui_item_lista(item, mensagem->remetente, estado_da_entrada(entrada, mensagem), i == game->caixa_selecionada,
                          cor_do_estado(entrada, mensagem)))
            game->caixa_selecionada = i;
        y += ITEM_ALTURA + 4;
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

void painel_receptor(GameState *game, Rectangle area) {
    desenhar_abas(game, area);
    if (game->receptor_em_novidades) {
        desenhar_novidades(game, area);
        return;
    }
    float y = desenhar_fila(game, area);
    desenhar_caixa(game, area, y);
    Rectangle coluna = {area.x + LISTA_LARGURA + 24, area.y + 44, area.width - LISTA_LARGURA - 24, area.height - 44};
    desenhar_mensagem(game, coluna);
}
