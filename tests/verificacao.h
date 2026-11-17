/* Mini framework de verificação compartilhado pelos testes (sem dependências externas). */
#ifndef VERIFICACAO_H
#define VERIFICACAO_H

#include <stdio.h>

static int falhas = 0;

#define VERIFICA(condicao)                                                     \
    do {                                                                       \
        if (!(condicao)) {                                                     \
            printf("  FALHOU %s:%d: %s\n", __FILE__, __LINE__, #condicao);     \
            falhas++;                                                          \
        }                                                                      \
    } while (0)

static int relatorio(const char *nome) {
    if (falhas) {
        printf("%s: %d verificação(ões) falharam\n", nome, falhas);
        return 1;
    }
    printf("%s: todos os testes passaram\n", nome);
    return 0;
}

#endif
