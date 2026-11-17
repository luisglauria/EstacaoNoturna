#include "configuracoes.h"
#include <stdio.h>
#include <string.h>

Configuracoes configuracoes_padrao(void) {
    return (Configuracoes){.volume_geral = 80, .volume_musica = 60, .volume_efeitos = 80, .tela_cheia = false, .dicas = true};
}

static int limitar(int valor, int padrao) { return (valor < 0 || valor > 100) ? padrao : valor; }

/* Aplica uma linha "chave=valor"; chaves desconhecidas são ignoradas. */
static void aplicar_linha(Configuracoes *configuracoes, const char *chave, int valor) {
    Configuracoes padrao = configuracoes_padrao();
    if (strcmp(chave, "volume_geral") == 0) configuracoes->volume_geral = limitar(valor, padrao.volume_geral);
    if (strcmp(chave, "volume_musica") == 0) configuracoes->volume_musica = limitar(valor, padrao.volume_musica);
    if (strcmp(chave, "volume_efeitos") == 0) configuracoes->volume_efeitos = limitar(valor, padrao.volume_efeitos);
    if (strcmp(chave, "tela_cheia") == 0) configuracoes->tela_cheia = valor != 0;
    if (strcmp(chave, "dicas") == 0) configuracoes->dicas = valor != 0;
}

Configuracoes configuracoes_carregar(const char *caminho) {
    Configuracoes configuracoes = configuracoes_padrao();
    FILE *arquivo = fopen(caminho, "r");
    if (!arquivo) return configuracoes;
    char chave[64];
    int valor;
    while (fscanf(arquivo, " %63[^=]=%d", chave, &valor) == 2) aplicar_linha(&configuracoes, chave, valor);
    fclose(arquivo);
    return configuracoes;
}

bool configuracoes_salvar(const Configuracoes *configuracoes, const char *caminho) {
    FILE *arquivo = fopen(caminho, "w");
    if (!arquivo) return false;
    fprintf(arquivo, "volume_geral=%d\nvolume_musica=%d\nvolume_efeitos=%d\ntela_cheia=%d\ndicas=%d\n",
            configuracoes->volume_geral, configuracoes->volume_musica, configuracoes->volume_efeitos,
            configuracoes->tela_cheia ? 1 : 0, configuracoes->dicas ? 1 : 0);
    fclose(arquivo);
    return true;
}
