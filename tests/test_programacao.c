/* Testes da lista duplamente encadeada da programação; rodam sem raylib com `make test`. */
#include "../programacao.h"
#include "verificacao.h"
#include <string.h>

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

/* Confere a ordem pelos códigos e também os ponteiros anterior, inicio, fim e tamanho. */
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
    VERIFICA(programacao.fim == corvo); /* o próprio nó mudou de lugar; os dados não foram copiados */
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

int main(void) {
    testeListaVazia();
    testeUmUnicoNo();
    testeInsercaoEBusca();
    testeOrdenacaoEstavelMoveNos();
    testeMovimentacao();
    testeRemocaoInicioMeioFim();
    testeExecutarProxima();
    testeExibirPercorreEmOrdem();
    return relatorio("programacao");
}
