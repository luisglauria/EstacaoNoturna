#ifndef PROGRAMACAO_H
#define PROGRAMACAO_H

#include <stdbool.h>

#define TEXTO_CODIGO 16
#define TEXTO_TITULO 96

/* Uma transmissão da programação da rádio: o conteúdo de um nó da lista. */
typedef struct {
    int id;                      /* id da mensagem da campanha que originou a transmissão */
    char codigo[TEXTO_CODIGO];   /* identificador curto exibido e usado em dependências (ex.: ORION) */
    char titulo[TEXTO_TITULO];   /* texto que vai ao ar */
    int custo;                   /* pontos de ação gastos ao transmitir */
    int prazo;                   /* última ação da noite em que a transmissão ainda funciona */
    char requer[TEXTO_CODIGO];   /* código que precisa ter ido ao ar antes, nesta noite ("" = nenhum) */
    unsigned consequencia;       /* marcas da campanha ganhas se der certo (contato protegido, prova...) */
    bool urgente;
} Transmissao;

typedef struct NoTransmissao {
    Transmissao dados;
    struct NoTransmissao *anterior;
    struct NoTransmissao *proximo;
} NoTransmissao;

/* Lista duplamente encadeada: a ordem dos nós é a ordem em que a rádio vai transmitir. */
typedef struct {
    NoTransmissao *inicio;
    NoTransmissao *fim;
    int tamanho;
} Programacao;

/* Chamado por exibirProgramacao para cada nó, do início ao fim. */
typedef void (*VisitanteTransmissao)(const NoTransmissao *no, int posicao, void *contexto);

/* Deixa a programação vazia, sem liberar nós (use liberarProgramacao para isso). */
void inicializarProgramacao(Programacao *programacao);
/* Insere a transmissão no fim da programação; devolve NULL se faltar memória. */
NoTransmissao *inserirTransmissao(Programacao *programacao, Transmissao dados);
/* Procura pelo código (ex.: "ORION"); devolve NULL se não houver. */
NoTransmissao *buscarTransmissao(const Programacao *programacao, const char *codigo);
/* Devolve o nó na posição (0 = início) ou NULL. */
NoTransmissao *buscarTransmissaoNaPosicao(const Programacao *programacao, int posicao);
/* Devolve a posição do nó (0 = início) ou -1 se ele não estiver na lista. */
int posicaoDaTransmissao(const Programacao *programacao, const NoTransmissao *no);
/* Tira o nó da lista e libera a memória; devolve false se o nó for NULL. */
bool removerTransmissao(Programacao *programacao, NoTransmissao *no);
/* Troca o nó com o vizinho (direcao -1 = sobe, +1 = desce) religando ponteiros; false se não puder. */
bool moverTransmissao(Programacao *programacao, NoTransmissao *no, int direcao);
/* Percorre a programação do início ao fim chamando o visitante (a interface desenha por aqui). */
void exibirProgramacao(const Programacao *programacao, VisitanteTransmissao visitante, void *contexto);
/* Insertion Sort estável pelo prazo (menor primeiro), movendo os nós em vez de copiar dados. */
void ordenarPorPrazo(Programacao *programacao);
/* Retira o primeiro nó, copia os dados para *executada e libera o nó; false se estiver vazia. */
bool executarProximaTransmissao(Programacao *programacao, Transmissao *executada);
/* Libera todos os nós e deixa a programação vazia. */
void liberarProgramacao(Programacao *programacao);

#endif
