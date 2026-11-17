/* Funções compartilhadas entre partida.c (fluxo e eventos) e partida_acoes.c (ações do jogador).
   Não faz parte da interface pública: a interface só inclui partida.h. */
#ifndef PARTIDA_INTERNO_H
#define PARTIDA_INTERNO_H

#include "partida.h"

/* Acrescenta uma linha ao aviso da ação atual (eventos da mesma ação se somam). */
void partida_avisar(Partida *partida, SomPartida som, const char *formato, ...);
void partida_limpar_aviso(Partida *partida);
/* Confere fase e pontos; se não der, explica no aviso o custo e o saldo. */
bool partida_pode_gastar(Partida *partida, int custo);
/* Gasta pontos e avança o relógio uma ação por ponto, disparando chegadas e emergências.
   Devolve false se a noite terminou no meio (captura). */
bool partida_gastar(Partida *partida, int custo);
/* Encerra a operação se os pontos acabaram. */
void partida_verificar_fim(Partida *partida);
void partida_encerrar(Partida *partida, MotivoFim motivo);
bool partida_resolver_inspecao(Partida *partida);
EntradaCaixa *partida_entrada(Partida *partida, int indice_caixa);
void partida_registrar_codigo(Partida *partida, const char *codigo);
void partida_somar_suspeita(Partida *partida, int quantidade);

#endif
