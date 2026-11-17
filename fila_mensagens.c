#include "fila_mensagens.h"
#include <stdlib.h>

void inicializarFila(FilaMensagens *fila) {
    fila->inicio = NULL;
    fila->fim = NULL;
    fila->tamanho = 0;
}

bool enfileirar(FilaMensagens *fila, int id_mensagem) {
    NoFila *no = malloc(sizeof(*no));
    if (!no) return false;
    no->id_mensagem = id_mensagem;
    no->proximo = NULL;
    if (fila->fim) fila->fim->proximo = no;
    else fila->inicio = no;
    fila->fim = no;
    fila->tamanho++;
    return true;
}

bool desenfileirar(FilaMensagens *fila, int *id_mensagem) {
    NoFila *primeiro = fila->inicio;
    if (!primeiro) return false;
    *id_mensagem = primeiro->id_mensagem;
    fila->inicio = primeiro->proximo;
    if (!fila->inicio) fila->fim = NULL;
    free(primeiro);
    fila->tamanho--;
    return true;
}

bool filaVazia(const FilaMensagens *fila) { return fila->inicio == NULL; }

void liberarFila(FilaMensagens *fila) {
    int descartada;
    while (desenfileirar(fila, &descartada)) {
    }
}
