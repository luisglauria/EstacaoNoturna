/* Bancada: preparativo da noite, documentos, investigação (1 PA) e comparação grátis com mensagens. */
#include "paineis.h"
#include "render.h"
#include "widgets.h"

#define LISTA_LARGURA 260
#define ITEM_ALTURA 38

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
        Rectangle item = {area.x, y, LISTA_LARGURA, ITEM_ALTURA};
        const char *detalhe = liberado ? (documento->noite == game->partida.noite ? "novo nesta noite" : "") : "ainda não obtido";
        if (ui_item_lista(item, liberado ? documento->titulo : "???", detalhe, game->documento_selecionado == documento->id,
                          liberado ? COLOR_SIGNAL : COLOR_AMBER))
            game->documento_selecionado = documento->id;
        y += ITEM_ALTURA + 4;
    }
}

/* Seletor "< mensagem >" e o botão Comparar (grátis). */
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
    Rectangle botao = {area.x, area.y + area.height - 40, LISTA_LARGURA, 40};
    if (partida->provas_escondidas) {
        render_text("Papéis escondidos sob o piso.", botao.x, botao.y + 12, 14, COLOR_SIGNAL);
        return;
    }
    if (ui_botao_custo(botao, "Esconder os papéis", partida_custo_esconder(partida), partida->fase == FASE_OPERACAO))
        partida_esconder_provas(partida);
}

void painel_bancada(GameState *game, Rectangle area) {
    float y = desenhar_preparativos(game, area);
    desenhar_lista_documentos(game, area, y);
    Rectangle coluna = {area.x + LISTA_LARGURA + 24, y + 20, area.width - LISTA_LARGURA - 24, area.y + area.height - y - 20};
    desenhar_documento(game, coluna);
    desenhar_esconderijo(game, area);
}
