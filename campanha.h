#ifndef CAMPANHA_H
#define CAMPANHA_H

/* Dados narrativos da campanha: textos, mensagens, documentos e tarefas das quatro noites.
   Só dados; as regras que os usam ficam em partida.c. Edite os textos em campanha.c. */

#include "programacao.h"
#include <stdbool.h>

#define TOTAL_NOITES 4
#define MAX_RESPOSTAS 3

/* Marcas da campanha: o que já aconteceu e vale para as noites seguintes (bits de um unsigned). */
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

/* Resposta possível a uma mensagem (ex.: compartilhar ou não a programação com o Farol). */
typedef struct {
    const char *rotulo;
    int custo;
    unsigned marca;
    const char *resultado;
} RespostaMensagem;

typedef struct {
    int id;
    int noite;
    int chegada;              /* ação em que entra na fila; 0 = início da operação; -1 = já na programação */
    unsigned exige_marca;     /* só aparece se todas estas marcas existirem */
    unsigned bloqueia_marca;  /* não aparece se alguma destas marcas existir */
    const char *remetente;
    const char *texto;
    Veracidade veracidade;
    const char *confirmacao;  /* o que o pedido de confirmação revela */
    int documento_contradiz;  /* id do documento que mostra a contradição, ou 0 (nenhum) */
    const char *contradicao;
    unsigned marca_ao_expor;  /* marca ganha quando a mensagem é desmascarada */
    bool transmissivel;
    Transmissao transmissao;  /* conteúdo do nó criado ao aceitar a mensagem */
    const char *resultado;    /* texto do que acontece quando vai ao ar */
    int total_respostas;
    RespostaMensagem respostas[MAX_RESPOSTAS];
} Mensagem;

typedef struct {
    int id;
    int noite;                /* a partir desta noite fica na bancada */
    bool exige_investigacao;  /* custa 1 PA para obter */
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
    META_TRANSMITIR,          /* codigo foi ao ar com sucesso */
    META_DESMASCARAR,         /* mensagem valor foi confirmada, contradita ou descartada */
    META_EVITAR,              /* codigo nunca foi ao ar */
    META_HOLOFOTE,
    META_INSPECAO,
    META_RASTREAMENTO_MAXIMO, /* rastreamento <= valor */
    META_MARCA,               /* marca valor existe */
    META_INVESTIGAR,          /* documento valor foi investigado */
    META_PROVAS,              /* pelo menos valor provas no ar */
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
    const char *falha;       /* consequência escrita se obrigatória e não feita */
    int suspeita_falha;
} Tarefa;

typedef struct {
    int x, y, largura, altura;
} AreaChao;

typedef struct {
    const char *titulo;
    const char *abertura;
    int pontos_acao;
    int holofote_chegada;    /* 0 = sem holofote */
    int holofote_aviso;      /* quantas ações antes o aviso aparece */
    AreaChao area_holofote;  /* parte do piso que o feixe ilumina */
    int inspecao_chegada;    /* 0 = sem inspeção */
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

#endif
