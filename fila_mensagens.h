#ifndef FILA_MENSAGENS_H
#define FILA_MENSAGENS_H

#include <stdbool.h>

/* Fila FIFO encadeada: mensagens chegando ao receptor, atendidas na ordem de chegada. */
typedef struct NoFila {
    int id_mensagem;
    struct NoFila *proximo;
} NoFila;

typedef struct {
    NoFila *inicio;  /* próxima a ser recebida */
    NoFila *fim;     /* última que chegou */
    int tamanho;
} FilaMensagens;

void inicializarFila(FilaMensagens *fila);
/* Coloca a mensagem no fim da fila; false se faltar memória. */
bool enfileirar(FilaMensagens *fila, int id_mensagem);
/* Tira a mensagem do início e devolve o id em *id_mensagem; false se a fila estiver vazia. */
bool desenfileirar(FilaMensagens *fila, int *id_mensagem);
bool filaVazia(const FilaMensagens *fila);
/* Libera todos os nós e deixa a fila vazia. */
void liberarFila(FilaMensagens *fila);

#endif
