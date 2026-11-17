#ifndef PARTIDA_H
#define PARTIDA_H

/* Regras do jogo (partida.c) e tipos dos dados das quatro noites (campanha.c).
   Não usa raylib, por isso os testes compilam só esta parte. */
#include "programacao.h"
#include <stdbool.h>

#define TOTAL_NOITES 4
#define MAX_RESPOSTAS 3

/* Marcas: o que já aconteceu na campanha e vale para as noites seguintes (um bit cada). */
#define MARCA_LINCE_AJUDADO (1u << 0)
#define MARCA_GARCA_AJUDADA (1u << 1)
#define MARCA_PROVA_LISTA (1u << 2)
#define MARCA_PROVA_FOTOS (1u << 3)
#define MARCA_PROVA_ESCUTAS (1u << 4)
#define MARCA_PROVA_ARQUIVOS (1u << 5)
#define MARCA_DENUNCIA_FINAL (1u << 6)
#define MARCA_DENUNCIA_PRECOCE (1u << 7)
#define MARCA_FAROL_COM_PROGRAMACAO (1u << 8)
#define MARCA_PROGRAMACAO_FALSA (1u << 9)
#define MARCA_CODIGO_RETIRADO (1u << 10)
#define MARCA_FAROL_DESMASCARADO (1u << 11)
#define MARCA_BENEFICIO_AVISO (1u << 12)
#define MARCA_BENEFICIO_CONFIRMACAO (1u << 13)
#define MARCA_BENEFICIO_ESCONDERIJO (1u << 14)
#define MARCAS_PROVAS (MARCA_PROVA_LISTA | MARCA_PROVA_FOTOS | MARCA_PROVA_ESCUTAS | MARCA_PROVA_ARQUIVOS)

typedef enum { VERACIDADE_VERDADEIRA, VERACIDADE_FALSA, VERACIDADE_INCOMPLETA } Veracidade;

typedef struct {
    const char *rotulo;
    int custo;
    unsigned marca;
    const char *resultado;
} RespostaMensagem;

typedef struct {
    int id;
    int noite;
/* ação em que a mensagem chega; -1 = já começa no roteiro */
    int chegada;
    unsigned exige_marca;
    unsigned bloqueia_marca;
    const char *remetente;
    const char *texto;
    Veracidade veracidade;
    const char *confirmacao;
    int documento_contradiz;
    const char *contradicao;
    unsigned marca_ao_expor;
    bool transmissivel;
    Transmissao transmissao;
    const char *resultado;
    int total_respostas;
    RespostaMensagem respostas[MAX_RESPOSTAS];
} Mensagem;

typedef struct {
    int id;
    int noite;
    bool exige_investigacao;
    const char *titulo;
    const char *texto;
} Documento;

typedef struct {
    int noite;
    unsigned exige_marca;
    unsigned bloqueia_marca;
    const char *titulo;
    const char *texto;
} Novidade;

typedef enum {
    META_LER_NOVIDADES,
    META_PREPARATIVO,
    META_INICIAR,
    META_TRANSMITIR,
    META_DESMASCARAR,
    META_EVITAR,
    META_HOLOFOTE,
    META_INSPECAO,
    META_RASTREAMENTO_MAXIMO,
    META_MARCA,
    META_INVESTIGAR,
    META_PROVAS,
    META_PROVAS_ESCONDIDAS
} TipoMeta;

typedef struct {
    int noite;
    bool obrigatoria;
    TipoMeta tipo;
    const char *codigo;
    int valor;
    unsigned exige_marca;
    unsigned bloqueia_marca;
    const char *texto;
    const char *dica;
    const char *falha;
    int suspeita_falha;
} Tarefa;

typedef struct {
    int x, y, largura, altura;
} AreaChao;

typedef struct {
    const char *titulo;
    const char *abertura;
    int pontos_acao;
    int holofote_chegada;
    int holofote_aviso;
    AreaChao area_holofote;
    int inspecao_chegada;
    int inspecao_aviso;
} DefinicaoNoite;

typedef enum { PREPARO_NENHUM, PREPARO_BATERIA, PREPARO_CORTINA, PREPARO_EMBARALHADOR, PREPARO_ESCONDERIJO, PREPARO_TOTAL } Preparativo;

typedef struct {
    const char *nome;
    const char *descricao;
} DescricaoPreparativo;

typedef enum { OBJETO_FOTO, OBJETO_BILHETE, OBJETO_RADIO, OBJETO_MARCAS, OBJETO_TOTAL } ObjetoNarrativo;

typedef struct {
    const char *titulo;
    const char *texto;
} TextoObjeto;

typedef enum { FINAL_NENHUM, FINAL_COMPLETA, FINAL_INCOMPLETA, FINAL_CAPTURA } Final;

typedef struct {
    const char *titulo;
    const char *texto;
} TextoFinal;

const DefinicaoNoite *campanha_noite(int numero);
const Mensagem *campanha_mensagens(int *quantidade);
const Mensagem *campanha_mensagem(int id);
const Documento *campanha_documentos(int *quantidade);
const Documento *campanha_documento(int id);
const Novidade *campanha_novidades(int *quantidade);
const Tarefa *campanha_tarefas(int *quantidade);
const DescricaoPreparativo *campanha_preparativo(Preparativo preparativo);
const TextoObjeto *campanha_objeto(ObjetoNarrativo objeto);
const TextoFinal *campanha_final(Final final);

#define MAX_CAIXA 16
#define MAX_REGISTROS 16
#define MAX_CODIGOS_NO_AR 12
#define MAX_CONSEQUENCIAS 8
#define MAX_DOCUMENTOS 16
#define TEXTO_AVISO 240

typedef enum { FASE_PREPARACAO, FASE_OPERACAO, FASE_ENCERRADA } Fase;
typedef enum { FIM_NENHUM, FIM_DORMIU, FIM_SEM_PONTOS, FIM_RASTREADO, FIM_CAPTURA } MotivoFim;
typedef enum { EMERGENCIA_INATIVA, EMERGENCIA_AGUARDANDO, EMERGENCIA_AVISADA, EMERGENCIA_RESOLVIDA } EstadoEmergencia;
typedef enum { SOM_NENHUM, SOM_CONFIRMA, SOM_ERRO, SOM_TRANSMISSAO, SOM_FALHA, SOM_MENSAGEM, SOM_PAPEL, SOM_ALERTA, SOM_HOLOFOTE, SOM_INSPECAO } SomPartida;

#define HORA_INICIAL_EM_MINUTOS (23 * 60)
#define MINUTOS_POR_ACAO 15

typedef struct {
    int id_mensagem;
    bool confirmada;
    bool contradicao;
    bool aceita;
    bool descartada;
    bool transmitida;
    int resposta;
} EntradaCaixa;

typedef struct {
    char codigo[TEXTO_CODIGO];
    char titulo[TEXTO_TITULO];
    bool sucesso;
    char resultado[TEXTO_AVISO];
} RegistroTransmissao;

/* Estado da partida. Cada ação deixa um texto em aviso e um som em som para a interface. */
typedef struct {
    int noite;
    unsigned marcas;
    int suspeita;
    Final final;
    bool documento_investigado[MAX_DOCUMENTOS];

    Fase fase;
    Preparativo preparativo;
    bool leu_novidades;
    int pontos_acao;
    int pontos_maximos;
/* ações já gastas na noite: é o tempo do jogo */
    int relogio;
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
    bool jogador_na_area_holofote;
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

/* ---------- funções usadas pela interface e pelos testes ---------- */
void partida_nova(Partida *partida);
void partida_comecar_noite(Partida *partida, int noite);
bool partida_proxima_noite(Partida *partida);
void partida_liberar(Partida *partida);

void partida_ler_novidades(Partida *partida);
bool partida_escolher_preparativo(Partida *partida, Preparativo preparativo);
bool partida_iniciar_operacao(Partida *partida);

bool partida_receber_mensagem(Partida *partida);
bool partida_confirmar(Partida *partida, int indice_caixa);
bool partida_aceitar(Partida *partida, int indice_caixa);
bool partida_descartar(Partida *partida, int indice_caixa);
bool partida_responder(Partida *partida, int indice_caixa, int resposta);
bool partida_comparar(Partida *partida, int indice_caixa, int id_documento);
bool partida_investigar(Partida *partida, int id_documento);
bool partida_esconder_provas(Partida *partida);

bool partida_remover_transmissao(Partida *partida, NoTransmissao *no);
bool partida_mover_transmissao(Partida *partida, NoTransmissao *no, int direcao);
void partida_priorizar(Partida *partida);
bool partida_transmitir_proxima(Partida *partida);
bool partida_trocar_frequencia(Partida *partida);
bool partida_alternar_cortina(Partida *partida);
bool partida_alternar_equipamentos(Partida *partida);
bool partida_responder_porta(Partida *partida);
bool partida_dormir(Partida *partida);

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
int partida_acoes_ate_holofote(const Partida *partida);
int partida_acoes_ate_inspecao(const Partida *partida);
void partida_formatar_hora(int acao, char saida[6]);
const Tarefa *partida_proxima_obrigatoria(const Partida *partida);

#endif
