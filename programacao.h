#ifndef PROGRAMACAO_H
#define PROGRAMACAO_H

/* Estruturas de dados do jogo: a programação da rádio (lista duplamente encadeada)
   e a fila de sinais que chegam ao receptor (fila FIFO). */
#include <stdbool.h>

#define TEXTO_CODIGO 16
#define TEXTO_TITULO 96

/* Dados de uma transmissão (o conteúdo de cada nó da lista). */
typedef struct {
    int id;
    char codigo[TEXTO_CODIGO];
    char titulo[TEXTO_TITULO];
    int custo;
/* última ação da noite em que a transmissão ainda vale */
    int prazo;
/* código que precisa ter ido ao ar antes ("" = nenhum) */
    char requer[TEXTO_CODIGO];
    unsigned consequencia;
    bool urgente;
} Transmissao;

typedef struct NoTransmissao {
    Transmissao dados;
    struct NoTransmissao *anterior;
    struct NoTransmissao *proximo;
} NoTransmissao;

/* A ordem dos nós é a ordem em que a rádio vai transmitir. */
typedef struct {
    NoTransmissao *inicio;
    NoTransmissao *fim;
    int tamanho;
} Programacao;

typedef void (*VisitanteTransmissao)(const NoTransmissao *no, int posicao, void *contexto);

void inicializarProgramacao(Programacao *programacao);
NoTransmissao *inserirTransmissao(Programacao *programacao, Transmissao dados);
NoTransmissao *buscarTransmissao(const Programacao *programacao, const char *codigo);
NoTransmissao *buscarTransmissaoNaPosicao(const Programacao *programacao, int posicao);
int posicaoDaTransmissao(const Programacao *programacao, const NoTransmissao *no);
bool removerTransmissao(Programacao *programacao, NoTransmissao *no);
bool moverTransmissao(Programacao *programacao, NoTransmissao *no, int direcao);
void exibirProgramacao(const Programacao *programacao, VisitanteTransmissao visitante, void *contexto);
void ordenarPorPrazo(Programacao *programacao);
bool executarProximaTransmissao(Programacao *programacao, Transmissao *executada);
void liberarProgramacao(Programacao *programacao);

/* Fila FIFO: os sinais são recebidos na ordem em que chegaram. */
typedef struct NoFila {
    int id_mensagem;
    struct NoFila *proximo;
} NoFila;

typedef struct {
    NoFila *inicio;
    NoFila *fim;
    int tamanho;
} FilaMensagens;

void inicializarFila(FilaMensagens *fila);
bool enfileirar(FilaMensagens *fila, int id_mensagem);
bool desenfileirar(FilaMensagens *fila, int *id_mensagem);
bool filaVazia(const FilaMensagens *fila);
void liberarFila(FilaMensagens *fila);

#endif
