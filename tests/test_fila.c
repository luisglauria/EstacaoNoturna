/* Testes da fila FIFO de mensagens do receptor. */
#include "../fila_mensagens.h"
#include "verificacao.h"

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
    enfileirar(&fila, 40); /* chegou depois: vai para o fim */
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
    enfileirar(&fila, 7); /* continua usável depois de liberada */
    int id;
    VERIFICA(desenfileirar(&fila, &id) && id == 7);
}

int main(void) {
    testeFilaVazia();
    testeOrdemDeChegada();
    testeLiberarComItens();
    return relatorio("fila_mensagens");
}
