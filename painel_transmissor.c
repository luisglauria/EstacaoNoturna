/* Transmissor: o roteiro da noite (a lista de transmissões) com ordem, horários-limite e senhas.
   A lista é percorrida por exibirProgramacao, que chama desenhar_no para cada transmissão. */
#include "paineis.h"
#include "render.h"
#include "widgets.h"
#include <stdio.h>
#include <string.h>

#define MAX_LINHAS 6
#define LINHA_X 240
#define LINHA_LARGURA 570
#define LINHA_ALTURA 50
#define LINHA_INTERVALO 16
#define LINHAS_TOPO 192
#define COLUNA_X 836
#define COLUNA_LARGURA 270

/* Contexto passado ao visitante: simula a noite enquanto desenha, para prever atrasos e senhas. */
typedef struct {
    GameState *game;
    int acao_prevista;                     /* relógio previsto ao fim de cada transmissão */
    char codigos_antes[16][TEXTO_CODIGO];  /* senhas que já terão ido ao ar antes desta */
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

/* Um fio ligando uma ficha à seguinte, como cabos de um painel de rádio. */
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

/* Visitante chamado por exibirProgramacao para cada transmissão, da primeira à última. */
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

/* ---------- ações ---------- */

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

void painel_transmissor(GameState *game, Rectangle area) {
    tratar_teclas(game);
    desenhar_roteiro(game, area);
    float y = desenhar_estado(&game->partida, area.y + 34);
    y = desenhar_botoes_roteiro(game, y);
    y = desenhar_acao_principal(game, y);
    desenhar_equipamentos(game, y);
}
