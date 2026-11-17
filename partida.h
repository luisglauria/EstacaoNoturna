#ifndef PARTIDA_H
#define PARTIDA_H

/* Regras do jogo: noites, pontos de ação, mensagens, investigação, transmissões e emergências.
   Não usa raylib, para poder ser testado com `make test`. A interface só chama estas funções
   e lê os campos; cada ação deixa um texto em `aviso` e um som sugerido em `som`. */

#include "campanha.h"
#include "fila_mensagens.h"
#include "programacao.h"
#include <stdbool.h>

#define MAX_CAIXA 16
#define MAX_REGISTROS 16
#define MAX_CODIGOS_NO_AR 12
#define MAX_CONSEQUENCIAS 8
#define MAX_DOCUMENTOS 16
#define TEXTO_AVISO 240

typedef enum { FASE_PREPARACAO, FASE_OPERACAO, FASE_ENCERRADA } Fase;
typedef enum { FIM_NENHUM, FIM_DORMIU, FIM_SEM_PONTOS, FIM_RASTREADO, FIM_CAPTURA } MotivoFim;
typedef enum { EMERGENCIA_INATIVA, EMERGENCIA_AGUARDANDO, EMERGENCIA_AVISADA, EMERGENCIA_RESOLVIDA } EstadoEmergencia;
/* Sons a partir de SOM_ALERTA têm prioridade quando vários eventos acontecem na mesma ação. */
typedef enum { SOM_NENHUM, SOM_CONFIRMA, SOM_ERRO, SOM_TRANSMISSAO, SOM_FALHA, SOM_MENSAGEM, SOM_PAPEL, SOM_ALERTA, SOM_HOLOFOTE, SOM_INSPECAO } SomPartida;

/* A noite começa às 23:00 e cada ação dura 15 minutos. */
#define HORA_INICIAL_EM_MINUTOS (23 * 60)
#define MINUTOS_POR_ACAO 15

/* Uma mensagem recebida, com o que o jogador já descobriu ou decidiu sobre ela. */
typedef struct {
    int id_mensagem;
    bool confirmada;
    bool contradicao;
    bool aceita;
    bool descartada;
    bool transmitida;
    int resposta; /* índice da resposta escolhida ou -1 */
} EntradaCaixa;

typedef struct {
    char codigo[TEXTO_CODIGO];
    char titulo[TEXTO_TITULO];
    bool sucesso;
    char resultado[TEXTO_AVISO];
} RegistroTransmissao;

typedef struct {
    /* Campanha: vale para todas as noites. */
    int noite;
    unsigned marcas;
    int suspeita;
    Final final;
    bool documento_investigado[MAX_DOCUMENTOS];

    /* Noite atual. */
    Fase fase;
    Preparativo preparativo;
    bool leu_novidades;
    int pontos_acao;
    int pontos_maximos;
    int relogio; /* ações já gastas nesta noite: é o "tempo" do jogo */
    int rastreamento;
    Programacao programacao;
    FilaMensagens fila;
    EntradaCaixa caixa[MAX_CAIXA];
    int total_caixa;
    char codigos_no_ar[MAX_CODIGOS_NO_AR][TEXTO_CODIGO];
    int total_codigos_no_ar;
    RegistroTransmissao registros[MAX_REGISTROS];
    int total_registros;

    bool cortina_fechada;
    bool cortina_reforcada_usada;
    EstadoEmergencia holofote;
    int holofote_chegada;
    int holofote_aviso_em;
    bool holofote_viu;
    bool jogador_na_area_holofote; /* atualizado pela interface a cada quadro */
    EstadoEmergencia inspecao;
    int inspecao_chegada;
    int inspecao_aviso_em;
    bool inspecao_superada;
    bool provas_escondidas;
    bool equipamentos_ligados;
    bool confirmacao_gratis;

    MotivoFim motivo_fim;
    char consequencias[MAX_CONSEQUENCIAS][TEXTO_AVISO];
    int total_consequencias;

    char aviso[TEXTO_AVISO];
    SomPartida som;
} Partida;

/* ---------- ciclo da campanha ---------- */
void partida_nova(Partida *partida);
void partida_comecar_noite(Partida *partida, int noite);
/* Avança para a próxima noite; false se a campanha acabou (final definido). */
bool partida_proxima_noite(Partida *partida);
void partida_liberar(Partida *partida);

/* ---------- preparação ---------- */
void partida_ler_novidades(Partida *partida);
bool partida_escolher_preparativo(Partida *partida, Preparativo preparativo);
bool partida_iniciar_operacao(Partida *partida);

/* ---------- receptor e bancada ---------- */
bool partida_receber_mensagem(Partida *partida);
bool partida_confirmar(Partida *partida, int indice_caixa);
bool partida_aceitar(Partida *partida, int indice_caixa);
bool partida_descartar(Partida *partida, int indice_caixa);
bool partida_responder(Partida *partida, int indice_caixa, int resposta);
bool partida_comparar(Partida *partida, int indice_caixa, int id_documento);
bool partida_investigar(Partida *partida, int id_documento);
bool partida_esconder_provas(Partida *partida);

/* ---------- transmissor, frequência, janela e porta ---------- */
bool partida_remover_transmissao(Partida *partida, NoTransmissao *no);
bool partida_mover_transmissao(Partida *partida, NoTransmissao *no, int direcao);
void partida_priorizar(Partida *partida);
bool partida_transmitir_proxima(Partida *partida);
bool partida_trocar_frequencia(Partida *partida);
bool partida_alternar_cortina(Partida *partida);
bool partida_alternar_equipamentos(Partida *partida);
bool partida_responder_porta(Partida *partida);
/* Encerra a operação por vontade do jogador (dormir no colchão). */
bool partida_dormir(Partida *partida);

/* ---------- consultas para a interface ---------- */
const DefinicaoNoite *partida_definicao(const Partida *partida);
const Mensagem *partida_mensagem_da_caixa(const Partida *partida, int indice_caixa);
bool partida_novidade_visivel(const Partida *partida, const Novidade *novidade);
bool partida_documento_na_bancada(const Partida *partida, const Documento *documento);
bool partida_documento_liberado(const Partida *partida, const Documento *documento);
bool partida_tarefa_visivel(const Partida *partida, const Tarefa *tarefa);
bool partida_tarefa_concluida(const Partida *partida, const Tarefa *tarefa);
bool partida_codigo_no_ar(const Partida *partida, const char *codigo);
int partida_total_provas(const Partida *partida);
int partida_custo_confirmacao(const Partida *partida);
int partida_custo_esconder(const Partida *partida);
/* Ações que faltam para o holofote/inspeção chegar, ou -1 se não houver aviso ativo. */
int partida_acoes_ate_holofote(const Partida *partida);
int partida_acoes_ate_inspecao(const Partida *partida);
/* Escreve em saida o horário ("23:45") correspondente a uma ação da noite. */
void partida_formatar_hora(int acao, char saida[6]);
/* Primeira tarefa obrigatória ainda pendente desta noite, ou NULL. */
const Tarefa *partida_proxima_obrigatoria(const Partida *partida);

#endif
