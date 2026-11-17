#ifndef CONFIGURACOES_H
#define CONFIGURACOES_H

#include <stdbool.h>

/* Arquivo onde as configurações ficam salvas, ao lado do executável (pasta em que o jogo roda). */
#define ARQUIVO_CONFIGURACOES "settings.cfg"

typedef struct {
    int volume_geral;   /* 0 a 100 */
    int volume_musica;
    int volume_efeitos;
    bool tela_cheia;
    bool dicas;         /* balões e dicas da próxima tarefa durante a noite */
} Configuracoes;

Configuracoes configuracoes_padrao(void);
/* Lê o arquivo; o que faltar ou vier inválido fica no valor padrão. */
Configuracoes configuracoes_carregar(const char *caminho);
bool configuracoes_salvar(const Configuracoes *configuracoes, const char *caminho);

#endif
