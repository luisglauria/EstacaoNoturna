#include "programacao.h"
#include <stdlib.h>
#include <string.h>

void inicializarProgramacao(Programacao *programacao) {
    programacao->inicio = NULL;
    programacao->fim = NULL;
    programacao->tamanho = 0;
}

/* Desliga o nó dos vizinhos sem liberar memória; base de remover, mover e ordenar. */
static void desligarNo(Programacao *programacao, NoTransmissao *no) {
    if (no->anterior) no->anterior->proximo = no->proximo;
    else programacao->inicio = no->proximo;
    if (no->proximo) no->proximo->anterior = no->anterior;
    else programacao->fim = no->anterior;
    no->anterior = NULL;
    no->proximo = NULL;
    programacao->tamanho--;
}

/* Liga o nó logo depois de ancora; ancora NULL significa "virar o novo início". */
static void ligarDepois(Programacao *programacao, NoTransmissao *ancora, NoTransmissao *no) {
    no->anterior = ancora;
    no->proximo = ancora ? ancora->proximo : programacao->inicio;
    if (no->proximo) no->proximo->anterior = no;
    else programacao->fim = no;
    if (ancora) ancora->proximo = no;
    else programacao->inicio = no;
    programacao->tamanho++;
}

NoTransmissao *inserirTransmissao(Programacao *programacao, Transmissao dados) {
    NoTransmissao *no = malloc(sizeof(*no));
    if (!no) return NULL;
    no->dados = dados;
    ligarDepois(programacao, programacao->fim, no);
    return no;
}

NoTransmissao *buscarTransmissao(const Programacao *programacao, const char *codigo) {
    for (NoTransmissao *no = programacao->inicio; no; no = no->proximo) {
        if (strcmp(no->dados.codigo, codigo) == 0) return no;
    }
    return NULL;
}

NoTransmissao *buscarTransmissaoNaPosicao(const Programacao *programacao, int posicao) {
    if (posicao < 0) return NULL;
    NoTransmissao *no = programacao->inicio;
    for (int i = 0; no && i < posicao; i++) no = no->proximo;
    return no;
}

int posicaoDaTransmissao(const Programacao *programacao, const NoTransmissao *no) {
    int posicao = 0;
    for (const NoTransmissao *atual = programacao->inicio; atual; atual = atual->proximo, posicao++) {
        if (atual == no) return posicao;
    }
    return -1;
}

bool removerTransmissao(Programacao *programacao, NoTransmissao *no) {
    if (!no) return false;
    desligarNo(programacao, no);
    free(no);
    return true;
}

bool moverTransmissao(Programacao *programacao, NoTransmissao *no, int direcao) {
    if (!no) return false;
    if (direcao < 0 && no->anterior) {
        NoTransmissao *ancora = no->anterior->anterior;
        desligarNo(programacao, no);
        ligarDepois(programacao, ancora, no);
        return true;
    }
    if (direcao > 0 && no->proximo) {
        NoTransmissao *ancora = no->proximo;
        desligarNo(programacao, no);
        ligarDepois(programacao, ancora, no);
        return true;
    }
    return false;
}

void exibirProgramacao(const Programacao *programacao, VisitanteTransmissao visitante, void *contexto) {
    int posicao = 0;
    for (const NoTransmissao *no = programacao->inicio; no; no = no->proximo, posicao++) {
        visitante(no, posicao, contexto);
    }
}

/* Anda para trás pelos ponteiros "anterior" até o último nó com prazo <= ao do nó; NULL = vira o início.
   O "<=" (e não "<") é o que deixa a ordenação estável: empates mantêm a ordem do jogador. */
static NoTransmissao *buscarAncoraOrdenada(NoTransmissao *no) {
    NoTransmissao *atual = no->anterior;
    while (atual && atual->dados.prazo > no->dados.prazo) atual = atual->anterior;
    return atual;
}

void ordenarPorPrazo(Programacao *programacao) {
    NoTransmissao *atual = programacao->inicio ? programacao->inicio->proximo : NULL;
    while (atual) {
        NoTransmissao *seguinte = atual->proximo;
        NoTransmissao *ancora = buscarAncoraOrdenada(atual);
        if (ancora != atual->anterior) {
            desligarNo(programacao, atual);
            ligarDepois(programacao, ancora, atual);
        }
        atual = seguinte;
    }
}

bool executarProximaTransmissao(Programacao *programacao, Transmissao *executada) {
    NoTransmissao *primeiro = programacao->inicio;
    if (!primeiro) return false;
    *executada = primeiro->dados;
    removerTransmissao(programacao, primeiro);
    return true;
}

void liberarProgramacao(Programacao *programacao) {
    NoTransmissao *no = programacao->inicio;
    while (no) {
        NoTransmissao *seguinte = no->proximo;
        free(no);
        no = seguinte;
    }
    inicializarProgramacao(programacao);
}
