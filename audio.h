#ifndef AUDIO_H
#define AUDIO_H

/* Sons e música gerados por código na inicialização (sem arquivos de áudio no projeto). */

#include "configuracoes.h"

typedef enum {
    EFEITO_CLIQUE,
    EFEITO_CONFIRMA,
    EFEITO_ERRO,
    EFEITO_TRANSMISSAO,
    EFEITO_FALHA,
    EFEITO_MENSAGEM,
    EFEITO_SIRENE,    /* holofote se aproximando */
    EFEITO_BATIDAS,   /* agentes batendo à porta */
    EFEITO_PAPEL,
    EFEITO_PASSO,
    EFEITO_TROVAO,
    EFEITO_TOTAL
} EfeitoSonoro;

void audio_iniciar(void);
void audio_encerrar(void);
/* Mantém a música em loop e aplica volumes; chamar uma vez por quadro. */
void audio_atualizar(const Configuracoes *configuracoes, float delta);
/* 0 = suspense de fundo; 1 = perigo iminente (a camada de tensão entra aos poucos). */
void audio_definir_tensao(float tensao);
void audio_tocar(EfeitoSonoro efeito);
void audio_tocar_clique(void);

#endif
