/* Testes das regras: ordem, prazo, dependência, mensagem falsa, emergências e finais. */
#include "../partida.h"
#include "verificacao.h"
#include <string.h>

/* Recebe todas as mensagens da fila (ação grátis). */
static void receber_tudo(Partida *partida) {
    while (!filaVazia(&partida->fila)) partida_receber_mensagem(partida);
}

static int indice_da_mensagem(const Partida *partida, int id_mensagem) {
    for (int i = 0; i < partida->total_caixa; i++) {
        if (partida->caixa[i].id_mensagem == id_mensagem) return i;
    }
    return -1;
}

static void iniciar(Partida *partida, Preparativo preparativo) {
    partida_ler_novidades(partida);
    partida_escolher_preparativo(partida, preparativo);
    VERIFICA(partida_iniciar_operacao(partida));
    receber_tudo(partida);
}

static void testeOperacaoExigePreparativo(void) {
    Partida partida;
    partida_nova(&partida);
    VERIFICA(partida.fase == FASE_PREPARACAO);
    VERIFICA(partida.programacao.tamanho == 2); /* ORION e ARQ herdados do antigo operador */
    VERIFICA(!partida_iniciar_operacao(&partida));
    VERIFICA(!partida_confirmar(&partida, 0)); /* sem operação, nada que custe pontos */
    partida_escolher_preparativo(&partida, PREPARO_BATERIA);
    VERIFICA(partida_iniciar_operacao(&partida));
    VERIFICA(partida.pontos_acao == 8); /* 6 + bateria */
    VERIFICA(partida.fila.tamanho == 1); /* só a mensagem do Lince chega no início */
    partida_liberar(&partida);
}

static void testeDependenciaExigeCodigoAntes(void) {
    Partida partida;
    partida_nova(&partida);
    iniciar(&partida, PREPARO_BATERIA);
    partida_aceitar(&partida, indice_da_mensagem(&partida, 3));
    partida_remover_transmissao(&partida, buscarTransmissao(&partida.programacao, "ARQ"));
    /* R-17 (prazo 5) vem antes de ORION (prazo 6) depois da ordenação: a dependência falha. */
    partida_priorizar(&partida);
    VERIFICA(strcmp(partida.programacao.inicio->dados.codigo, "R-17") == 0);
    partida_transmitir_proxima(&partida);
    VERIFICA(!partida.registros[0].sucesso);
    VERIFICA(!(partida.marcas & MARCA_LINCE_AJUDADO));
    partida_liberar(&partida);
}

static void testeOrdemCertaDaSucessoEAvancaTempo(void) {
    Partida partida;
    partida_nova(&partida);
    iniciar(&partida, PREPARO_BATERIA);
    partida_aceitar(&partida, indice_da_mensagem(&partida, 3));
    partida_remover_transmissao(&partida, buscarTransmissao(&partida.programacao, "ARQ"));
    partida_transmitir_proxima(&partida); /* ORION */
    partida_transmitir_proxima(&partida); /* R-17 */
    VERIFICA(partida.registros[0].sucesso && partida.registros[1].sucesso);
    VERIFICA(partida.marcas & MARCA_LINCE_AJUDADO);
    VERIFICA(partida.relogio == 3 && partida.pontos_acao == 5);
    VERIFICA(partida.rastreamento == 30);
    VERIFICA(partida.fila.tamanho == 2); /* Farol (ação 1) e Vega-7 (ação 2) chegaram com o tempo */
    partida_liberar(&partida);
}

static void testePrazoVencido(void) {
    Partida partida;
    partida_nova(&partida);
    iniciar(&partida, PREPARO_BATERIA);
    partida_aceitar(&partida, indice_da_mensagem(&partida, 3));
    for (int i = 0; i < 4; i++) partida_trocar_frequencia(&partida); /* gasta 4 ações */
    partida_transmitir_proxima(&partida); /* ORION na ação 5 */
    partida_remover_transmissao(&partida, buscarTransmissao(&partida.programacao, "ARQ"));
    partida_transmitir_proxima(&partida); /* R-17 terminaria na ação 7 > prazo 5 */
    VERIFICA(partida.total_registros == 2 && !partida.registros[1].sucesso);
    VERIFICA(strstr(partida.registros[1].resultado, "Tarde") != NULL);
    partida_liberar(&partida);
}

static void testeContradicaoEArmadilha(void) {
    Partida partida;
    partida_nova(&partida);
    iniciar(&partida, PREPARO_BATERIA);
    partida_trocar_frequencia(&partida);
    partida_trocar_frequencia(&partida);
    receber_tudo(&partida);
    int vega = indice_da_mensagem(&partida, 5);
    VERIFICA(vega >= 0);
    VERIFICA(!partida_comparar(&partida, vega, 2));  /* mapa não contradiz */
    VERIFICA(partida_comparar(&partida, vega, 1));   /* caderno: VEGA descoberto */
    VERIFICA(partida.caixa[vega].contradicao);
    int suspeita_antes = partida.suspeita;
    partida_aceitar(&partida, vega);
    while (partida.programacao.inicio && strcmp(partida.programacao.inicio->dados.codigo, "VEGA") != 0) {
        partida_remover_transmissao(&partida, partida.programacao.inicio);
    }
    partida_transmitir_proxima(&partida);
    VERIFICA(partida.suspeita == suspeita_antes + 20);
    partida_liberar(&partida);
}

static void testeFimPorPontosEConsequencia(void) {
    Partida partida;
    partida_nova(&partida);
    iniciar(&partida, PREPARO_EMBARALHADOR);
    for (int i = 0; i < 6; i++) partida_trocar_frequencia(&partida);
    VERIFICA(partida.fase == FASE_ENCERRADA && partida.motivo_fim == FIM_SEM_PONTOS);
    VERIFICA(partida.total_consequencias >= 1); /* R-17 obrigatória não foi feita */
    VERIFICA(partida.suspeita == 5);
    VERIFICA(partida_proxima_noite(&partida) && partida.noite == 2);
    partida_liberar(&partida);
}

static void testeHolofoteVeJogadorNaArea(void) {
    Partida partida;
    partida_nova(&partida);
    partida_comecar_noite(&partida, 2);
    iniciar(&partida, PREPARO_BATERIA);
    partida.jogador_na_area_holofote = true;
    VERIFICA(partida.holofote_chegada == 5);
    for (int i = 0; i < 3; i++) partida_trocar_frequencia(&partida);
    VERIFICA(partida_acoes_ate_holofote(&partida) == 2);
    partida_trocar_frequencia(&partida);
    partida_trocar_frequencia(&partida);
    VERIFICA(partida.holofote == EMERGENCIA_RESOLVIDA && partida.holofote_viu);
    VERIFICA(partida.suspeita == 25);
    partida_liberar(&partida);
}

static void testeCortinaProtegeDoHolofote(void) {
    Partida partida;
    partida_nova(&partida);
    partida_comecar_noite(&partida, 2);
    iniciar(&partida, PREPARO_BATERIA);
    partida.jogador_na_area_holofote = true;
    partida_alternar_cortina(&partida);
    for (int i = 0; i < 4; i++) partida_trocar_frequencia(&partida);
    VERIFICA(partida.holofote == EMERGENCIA_RESOLVIDA && !partida.holofote_viu);
    VERIFICA(partida.suspeita == 0);
    partida_liberar(&partida);
}

static void testeInspecaoComProvasExpostasCaptura(void) {
    Partida partida;
    partida_nova(&partida);
    partida_comecar_noite(&partida, 4);
    iniciar(&partida, PREPARO_BATERIA);
    for (int i = 0; i < 3 && partida.fase == FASE_OPERACAO; i++) partida_trocar_frequencia(&partida);
    VERIFICA(partida.inspecao == EMERGENCIA_AVISADA);
    VERIFICA(partida_responder_porta(&partida));
    VERIFICA(partida.final == FINAL_CAPTURA && partida.fase == FASE_ENCERRADA);
    partida_liberar(&partida);
}

static void testeInspecaoPreparadaEFinalCompleta(void) {
    Partida partida;
    partida_nova(&partida);
    partida.marcas |= MARCA_PROVA_LISTA | MARCA_PROVA_FOTOS | MARCA_PROVA_ESCUTAS;
    partida_comecar_noite(&partida, 4);
    iniciar(&partida, PREPARO_ESCONDERIJO);
    partida_aceitar(&partida, indice_da_mensagem(&partida, 17));
    partida_transmitir_proxima(&partida); /* LUME */
    partida_trocar_frequencia(&partida);
    receber_tudo(&partida);
    partida_aceitar(&partida, indice_da_mensagem(&partida, 19));
    partida_transmitir_proxima(&partida); /* DENÚNCIA (3 PA) */
    VERIFICA(partida.marcas & MARCA_DENUNCIA_FINAL);
    VERIFICA(partida.inspecao == EMERGENCIA_AVISADA && partida_acoes_ate_inspecao(&partida) == 1);
    partida_alternar_equipamentos(&partida); /* desliga: 1 PA, e nessa ação os agentes chegam */
    VERIFICA(partida.inspecao == EMERGENCIA_RESOLVIDA && partida.inspecao_superada);
    partida_dormir(&partida);
    VERIFICA(partida.final == FINAL_COMPLETA);
    partida_liberar(&partida);
}

static void testeProgramacaoFalsaAtrasaInspecao(void) {
    Partida partida;
    partida_nova(&partida);
    partida.marcas |= MARCA_FAROL_COM_PROGRAMACAO | MARCA_PROGRAMACAO_FALSA;
    partida_comecar_noite(&partida, 4);
    iniciar(&partida, PREPARO_BATERIA);
    VERIFICA(partida.inspecao_chegada == 7);
    partida_liberar(&partida);

    partida_nova(&partida);
    partida.marcas |= MARCA_FAROL_COM_PROGRAMACAO;
    partida_comecar_noite(&partida, 4);
    iniciar(&partida, PREPARO_BATERIA);
    VERIFICA(partida.inspecao_chegada == 4);
    partida_liberar(&partida);
}

static void testeRastreamentoEncerraNoite(void) {
    Partida partida;
    partida_nova(&partida);
    iniciar(&partida, PREPARO_BATERIA);
    partida.rastreamento = 90;
    partida_transmitir_proxima(&partida);
    VERIFICA(partida.rastreamento == 100 && partida.motivo_fim == FIM_RASTREADO);
    VERIFICA(partida.fase == FASE_ENCERRADA && partida.final == FINAL_NENHUM);
    partida_liberar(&partida);
}

int main(void) {
    testeOperacaoExigePreparativo();
    testeDependenciaExigeCodigoAntes();
    testeOrdemCertaDaSucessoEAvancaTempo();
    testePrazoVencido();
    testeContradicaoEArmadilha();
    testeFimPorPontosEConsequencia();
    testeHolofoteVeJogadorNaArea();
    testeCortinaProtegeDoHolofote();
    testeInspecaoComProvasExpostasCaptura();
    testeInspecaoPreparadaEFinalCompleta();
    testeProgramacaoFalsaAtrasaInspecao();
    testeRastreamentoEncerraNoite();
    return relatorio("partida");
}
