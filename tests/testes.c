#include "../partida.h"
#include <stdio.h>
#include <string.h>

/* Testes da lista, da fila e das regras. Rodam sem raylib com make test. */
static int falhas = 0;

#define VERIFICA(condicao)                                                     \
    do {                                                                       \
        if (!(condicao)) {                                                     \
            printf("  FALHOU %s:%d: %s\n", __FILE__, __LINE__, #condicao);     \
            falhas++;                                                          \
        }                                                                      \
    } while (0)

static int relatorio(void) {
    if (falhas) {
        printf("%d verificação(ões) falharam\n", falhas);
        return 1;
    }
    printf("todos os testes passaram\n");
    return 0;
}

/* ---------- lista duplamente encadeada ---------- */
static Transmissao criar(const char *codigo, int prazo) {
    Transmissao transmissao = {0};
    snprintf(transmissao.codigo, sizeof(transmissao.codigo), "%s", codigo);
    transmissao.prazo = prazo;
    transmissao.custo = 1;
    return transmissao;
}

static Programacao criarLista(const char *codigos[], const int prazos[], int quantidade) {
    Programacao programacao;
    inicializarProgramacao(&programacao);
    for (int i = 0; i < quantidade; i++) inserirTransmissao(&programacao, criar(codigos[i], prazos[i]));
    return programacao;
}

static void verificarOrdem(const Programacao *programacao, const char *esperado[], int quantidade) {
    VERIFICA(programacao->tamanho == quantidade);
    const NoTransmissao *anterior = NULL;
    const NoTransmissao *no = programacao->inicio;
    for (int i = 0; i < quantidade; i++) {
        VERIFICA(no != NULL);
        if (!no) return;
        VERIFICA(strcmp(no->dados.codigo, esperado[i]) == 0);
        VERIFICA(no->anterior == anterior);
        anterior = no;
        no = no->proximo;
    }
    VERIFICA(no == NULL);
    VERIFICA(programacao->fim == anterior);
}

static void testeListaVazia(void) {
    Programacao programacao;
    inicializarProgramacao(&programacao);
    Transmissao saida;
    VERIFICA(!executarProximaTransmissao(&programacao, &saida));
    VERIFICA(buscarTransmissao(&programacao, "X") == NULL);
    VERIFICA(!removerTransmissao(&programacao, NULL));
    VERIFICA(!moverTransmissao(&programacao, NULL, 1));
    ordenarPorPrazo(&programacao);
    liberarProgramacao(&programacao);
    VERIFICA(programacao.inicio == NULL && programacao.fim == NULL && programacao.tamanho == 0);
}

static void testeUmUnicoNo(void) {
    const char *codigos[] = {"SO"};
    const int prazos[] = {4};
    Programacao programacao = criarLista(codigos, prazos, 1);
    VERIFICA(programacao.inicio == programacao.fim);
    VERIFICA(!moverTransmissao(&programacao, programacao.inicio, -1));
    VERIFICA(!moverTransmissao(&programacao, programacao.inicio, 1));
    ordenarPorPrazo(&programacao);
    verificarOrdem(&programacao, codigos, 1);
    VERIFICA(removerTransmissao(&programacao, programacao.inicio));
    VERIFICA(programacao.inicio == NULL && programacao.fim == NULL && programacao.tamanho == 0);
}

static void testeInsercaoEBusca(void) {
    const char *codigos[] = {"A", "B", "C"};
    const int prazos[] = {1, 2, 3};
    Programacao programacao = criarLista(codigos, prazos, 3);
    verificarOrdem(&programacao, codigos, 3);
    VERIFICA(buscarTransmissao(&programacao, "B") == programacao.inicio->proximo);
    VERIFICA(buscarTransmissao(&programacao, "Z") == NULL);
    VERIFICA(buscarTransmissaoNaPosicao(&programacao, 2) == programacao.fim);
    VERIFICA(buscarTransmissaoNaPosicao(&programacao, 3) == NULL);
    VERIFICA(posicaoDaTransmissao(&programacao, programacao.fim) == 2);
    liberarProgramacao(&programacao);
}

static void testeOrdenacaoEstavelMoveNos(void) {
    const char *codigos[] = {"CORVO", "PADARIA", "LISTA", "EMPATE"};
    const int prazos[] = {8, 3, 5, 3};
    Programacao programacao = criarLista(codigos, prazos, 4);
    NoTransmissao *corvo = programacao.inicio;
    ordenarPorPrazo(&programacao);
    const char *esperado[] = {"PADARIA", "EMPATE", "LISTA", "CORVO"};
    verificarOrdem(&programacao, esperado, 4);
    VERIFICA(programacao.fim == corvo);
    liberarProgramacao(&programacao);
}

static void testeMovimentacao(void) {
    const char *codigos[] = {"A", "B", "C"};
    const int prazos[] = {1, 2, 3};
    Programacao programacao = criarLista(codigos, prazos, 3);
    VERIFICA(moverTransmissao(&programacao, programacao.fim, -1));
    const char *subiu[] = {"A", "C", "B"};
    verificarOrdem(&programacao, subiu, 3);
    VERIFICA(moverTransmissao(&programacao, programacao.inicio->proximo, -1));
    const char *virou_inicio[] = {"C", "A", "B"};
    verificarOrdem(&programacao, virou_inicio, 3);
    VERIFICA(!moverTransmissao(&programacao, programacao.inicio, -1));
    VERIFICA(moverTransmissao(&programacao, programacao.inicio, 1));
    VERIFICA(!moverTransmissao(&programacao, programacao.fim, 1));
    const char *desceu[] = {"A", "C", "B"};
    verificarOrdem(&programacao, desceu, 3);
    liberarProgramacao(&programacao);
}

static void testeRemocaoInicioMeioFim(void) {
    const char *codigos[] = {"A", "B", "C", "D"};
    const int prazos[] = {1, 2, 3, 4};
    Programacao programacao = criarLista(codigos, prazos, 4);
    removerTransmissao(&programacao, programacao.inicio->proximo);
    const char *sem_b[] = {"A", "C", "D"};
    verificarOrdem(&programacao, sem_b, 3);
    removerTransmissao(&programacao, programacao.inicio);
    removerTransmissao(&programacao, programacao.fim);
    const char *so_c[] = {"C"};
    verificarOrdem(&programacao, so_c, 1);
    liberarProgramacao(&programacao);
}

static void testeExecutarProxima(void) {
    const char *codigos[] = {"ORION", "R-17"};
    const int prazos[] = {6, 5};
    Programacao programacao = criarLista(codigos, prazos, 2);
    Transmissao executada;
    VERIFICA(executarProximaTransmissao(&programacao, &executada));
    VERIFICA(strcmp(executada.codigo, "ORION") == 0);
    const char *restante[] = {"R-17"};
    verificarOrdem(&programacao, restante, 1);
    VERIFICA(executarProximaTransmissao(&programacao, &executada));
    VERIFICA(programacao.tamanho == 0 && programacao.inicio == NULL);
}

static void contarVisitas(const NoTransmissao *no, int posicao, void *contexto) {
    int *soma = contexto;
    *soma += (posicao + 1) * no->dados.prazo;
}

static void testeExibirPercorreEmOrdem(void) {
    const char *codigos[] = {"A", "B", "C"};
    const int prazos[] = {1, 10, 100};
    Programacao programacao = criarLista(codigos, prazos, 3);
    int soma = 0;
    exibirProgramacao(&programacao, contarVisitas, &soma);
    VERIFICA(soma == 1 * 1 + 2 * 10 + 3 * 100);
    liberarProgramacao(&programacao);
}

/* ---------- fila ---------- */
static void testeFilaVazia(void) {
    FilaMensagens fila;
    inicializarFila(&fila);
    int id = -1;
    VERIFICA(filaVazia(&fila));
    VERIFICA(!desenfileirar(&fila, &id));
    VERIFICA(id == -1);
    liberarFila(&fila);
    VERIFICA(fila.tamanho == 0);
}

static void testeOrdemDeChegada(void) {
    FilaMensagens fila;
    inicializarFila(&fila);
    enfileirar(&fila, 10);
    enfileirar(&fila, 20);
    enfileirar(&fila, 30);
    VERIFICA(fila.tamanho == 3);
    int id;
    VERIFICA(desenfileirar(&fila, &id) && id == 10);
    enfileirar(&fila, 40);
    VERIFICA(desenfileirar(&fila, &id) && id == 20);
    VERIFICA(desenfileirar(&fila, &id) && id == 30);
    VERIFICA(desenfileirar(&fila, &id) && id == 40);
    VERIFICA(filaVazia(&fila) && fila.fim == NULL && fila.tamanho == 0);
}

static void testeLiberarComItens(void) {
    FilaMensagens fila;
    inicializarFila(&fila);
    for (int i = 0; i < 5; i++) enfileirar(&fila, i);
    liberarFila(&fila);
    VERIFICA(filaVazia(&fila) && fila.fim == NULL && fila.tamanho == 0);
    enfileirar(&fila, 7);
    int id;
    VERIFICA(desenfileirar(&fila, &id) && id == 7);
}

/* ---------- regras da partida ---------- */
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
    VERIFICA(partida.programacao.tamanho == 2);
    VERIFICA(!partida_iniciar_operacao(&partida));
    VERIFICA(!partida_confirmar(&partida, 0));
    partida_escolher_preparativo(&partida, PREPARO_BATERIA);
    VERIFICA(partida_iniciar_operacao(&partida));
    VERIFICA(partida.pontos_acao == 8);
    VERIFICA(partida.fila.tamanho == 1);
    partida_liberar(&partida);
}

static void testeDependenciaExigeCodigoAntes(void) {
    Partida partida;
    partida_nova(&partida);
    iniciar(&partida, PREPARO_BATERIA);
    partida_aceitar(&partida, indice_da_mensagem(&partida, 3));
    partida_remover_transmissao(&partida, buscarTransmissao(&partida.programacao, "ARQ"));
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
    partida_transmitir_proxima(&partida);
    partida_transmitir_proxima(&partida);
    VERIFICA(partida.registros[0].sucesso && partida.registros[1].sucesso);
    VERIFICA(partida.marcas & MARCA_LINCE_AJUDADO);
    VERIFICA(partida.relogio == 3 && partida.pontos_acao == 5);
    VERIFICA(partida.rastreamento == 30);
    VERIFICA(partida.fila.tamanho == 2);
    partida_liberar(&partida);
}

static void testePrazoVencido(void) {
    Partida partida;
    partida_nova(&partida);
    iniciar(&partida, PREPARO_BATERIA);
    partida_aceitar(&partida, indice_da_mensagem(&partida, 3));
    for (int i = 0; i < 4; i++) partida_trocar_frequencia(&partida);
    partida_transmitir_proxima(&partida);
    partida_remover_transmissao(&partida, buscarTransmissao(&partida.programacao, "ARQ"));
    partida_transmitir_proxima(&partida);
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
    VERIFICA(!partida_comparar(&partida, vega, 2));
    VERIFICA(partida_comparar(&partida, vega, 1));
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
    VERIFICA(partida.total_consequencias >= 1);
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
    partida_transmitir_proxima(&partida);
    partida_trocar_frequencia(&partida);
    receber_tudo(&partida);
    partida_aceitar(&partida, indice_da_mensagem(&partida, 19));
    partida_transmitir_proxima(&partida);
    VERIFICA(partida.marcas & MARCA_DENUNCIA_FINAL);
    VERIFICA(partida.inspecao == EMERGENCIA_AVISADA && partida_acoes_ate_inspecao(&partida) == 1);
    partida_alternar_equipamentos(&partida);
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

static void testeRiscarTiraONoDaMensagemCerta(void) {
    Partida partida;
    partida_nova(&partida);
    Transmissao outra = {.id = 99, .codigo = "ORION", .custo = 1, .prazo = 6};
    NoTransmissao *no = inserirTransmissao(&partida.programacao, outra);
    moverTransmissao(&partida.programacao, no, -1);
    moverTransmissao(&partida.programacao, no, -1);
    VERIFICA(partida.programacao.inicio->dados.id == 99);
    VERIFICA(partida_descartar(&partida, indice_da_mensagem(&partida, 1)));
    VERIFICA(partida.programacao.tamanho == 2);
    VERIFICA(partida.programacao.inicio->dados.id == 99 && partida.programacao.fim->dados.id == 2);
    partida_liberar(&partida);
}

static void testeCapturaNoMeioDaTransmissaoFicaRegistrada(void) {
    Partida partida;
    partida_nova(&partida);
    partida_comecar_noite(&partida, 4);
    iniciar(&partida, PREPARO_BATERIA);
    for (int i = 0; i < 5; i++) partida_trocar_frequencia(&partida);
    partida_aceitar(&partida, indice_da_mensagem(&partida, 17));
    partida_transmitir_proxima(&partida);
    VERIFICA(partida.final == FINAL_CAPTURA && partida.fase == FASE_ENCERRADA);
    VERIFICA(partida.total_registros == 1 && !partida.registros[0].sucesso);
    partida_liberar(&partida);
}

int main(void) {
    testeListaVazia();
    testeUmUnicoNo();
    testeInsercaoEBusca();
    testeOrdenacaoEstavelMoveNos();
    testeMovimentacao();
    testeRemocaoInicioMeioFim();
    testeExecutarProxima();
    testeExibirPercorreEmOrdem();
    testeFilaVazia();
    testeOrdemDeChegada();
    testeLiberarComItens();
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
    testeRiscarTiraONoDaMensagemCerta();
    testeCapturaNoMeioDaTransmissaoFicaRegistrada();
    return relatorio();
}
