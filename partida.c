/* Fluxo da campanha e da noite: começo, relógio de ações, chegadas, emergências e encerramento. */
#include "partida_interno.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#define SUSPEITA_MAXIMA 100
#define RISCO_CAPTURA 70        /* risco da inspeção a partir do qual os agentes acham a rádio */
#define PROVAS_PARA_COMPLETA 3

/* ---------- utilidades internas ---------- */

void partida_limpar_aviso(Partida *partida) {
    partida->aviso[0] = '\0';
    partida->som = SOM_NENHUM;
}

void partida_avisar(Partida *partida, SomPartida som, const char *formato, ...) {
    size_t usado = strlen(partida->aviso);
    if (usado > 0 && usado + 1 < sizeof(partida->aviso)) partida->aviso[usado++] = '\n';
    va_list argumentos;
    va_start(argumentos, formato);
    vsnprintf(partida->aviso + usado, sizeof(partida->aviso) - usado, formato, argumentos);
    va_end(argumentos);
    /* Alerta de emergência tem prioridade sobre o som da ação comum. */
    if (partida->som == SOM_NENHUM || som >= SOM_ALERTA) partida->som = som;
}

void partida_somar_suspeita(Partida *partida, int quantidade) {
    partida->suspeita += quantidade;
    if (partida->suspeita > SUSPEITA_MAXIMA) partida->suspeita = SUSPEITA_MAXIMA;
    if (partida->suspeita < 0) partida->suspeita = 0;
}

static void adicionar_consequencia(Partida *partida, const char *texto) {
    if (!texto || !texto[0] || partida->total_consequencias >= MAX_CONSEQUENCIAS) return;
    snprintf(partida->consequencias[partida->total_consequencias++], TEXTO_AVISO, "%s", texto);
}

EntradaCaixa *partida_entrada(Partida *partida, int indice_caixa) {
    if (indice_caixa < 0 || indice_caixa >= partida->total_caixa) return NULL;
    return &partida->caixa[indice_caixa];
}

void partida_registrar_codigo(Partida *partida, const char *codigo) {
    if (partida_codigo_no_ar(partida, codigo) || partida->total_codigos_no_ar >= MAX_CODIGOS_NO_AR) return;
    snprintf(partida->codigos_no_ar[partida->total_codigos_no_ar++], TEXTO_CODIGO, "%s", codigo);
}

/* ---------- campanha ---------- */

void partida_nova(Partida *partida) {
    memset(partida, 0, sizeof(*partida));
    inicializarProgramacao(&partida->programacao);
    inicializarFila(&partida->fila);
    partida_comecar_noite(partida, 1);
}

static bool marcas_permitem(const Partida *partida, unsigned exige, unsigned bloqueia) {
    return (partida->marcas & exige) == exige && (partida->marcas & bloqueia) == 0;
}

static bool mensagem_desta_noite(const Partida *partida, const Mensagem *mensagem) {
    return mensagem->noite == partida->noite &&
           marcas_permitem(partida, mensagem->exige_marca, mensagem->bloqueia_marca);
}

static void adicionar_na_caixa(Partida *partida, int id_mensagem, bool aceita) {
    if (partida->total_caixa >= MAX_CAIXA) return;
    EntradaCaixa entrada = {.id_mensagem = id_mensagem, .aceita = aceita, .resposta = -1};
    partida->caixa[partida->total_caixa++] = entrada;
}

/* Transmissões que o antigo operador já deixou na programação (chegada -1). */
static void carregar_programacao_herdada(Partida *partida) {
    int total;
    const Mensagem *mensagens = campanha_mensagens(&total);
    for (int i = 0; i < total; i++) {
        if (mensagens[i].chegada != -1 || !mensagem_desta_noite(partida, &mensagens[i])) continue;
        inserirTransmissao(&partida->programacao, mensagens[i].transmissao);
        adicionar_na_caixa(partida, mensagens[i].id, true);
    }
}

static void zerar_estado_da_noite(Partida *partida) {
    liberarProgramacao(&partida->programacao);
    liberarFila(&partida->fila);
    partida->fase = FASE_PREPARACAO;
    partida->preparativo = PREPARO_NENHUM;
    partida->leu_novidades = false;
    partida->relogio = 0;
    partida->rastreamento = 0;
    partida->total_caixa = 0;
    partida->total_codigos_no_ar = 0;
    partida->total_registros = 0;
    partida->total_consequencias = 0;
    partida->motivo_fim = FIM_NENHUM;
}

static void zerar_emergencias(Partida *partida) {
    partida->cortina_fechada = false;
    partida->cortina_reforcada_usada = false;
    partida->holofote = EMERGENCIA_INATIVA;
    partida->holofote_viu = false;
    partida->jogador_na_area_holofote = false;
    partida->inspecao = EMERGENCIA_INATIVA;
    partida->inspecao_superada = false;
    partida->provas_escondidas = false;
    partida->equipamentos_ligados = true;
}

void partida_comecar_noite(Partida *partida, int noite) {
    zerar_estado_da_noite(partida);
    zerar_emergencias(partida);
    partida->noite = noite;
    partida->pontos_maximos = partida_definicao(partida)->pontos_acao;
    partida->pontos_acao = partida->pontos_maximos;
    partida->confirmacao_gratis = (partida->marcas & MARCA_BENEFICIO_CONFIRMACAO) != 0;
    carregar_programacao_herdada(partida);
    partida_limpar_aviso(partida);
}

bool partida_proxima_noite(Partida *partida) {
    if (partida->final != FINAL_NENHUM || partida->noite >= TOTAL_NOITES) return false;
    partida_comecar_noite(partida, partida->noite + 1);
    return true;
}

void partida_liberar(Partida *partida) {
    liberarProgramacao(&partida->programacao);
    liberarFila(&partida->fila);
}

/* ---------- início da operação ---------- */

/* Se o Farol tem a programação real, a polícia chega antes; a programação falsa a despista. */
static bool farol_com_programacao_real(const Partida *partida) {
    return (partida->marcas & MARCA_FAROL_COM_PROGRAMACAO) && !(partida->marcas & MARCA_PROGRAMACAO_FALSA);
}

static void agendar_holofote(Partida *partida, const DefinicaoNoite *definicao) {
    if (definicao->holofote_chegada <= 0) return;
    int chegada = definicao->holofote_chegada - (farol_com_programacao_real(partida) ? 1 : 0);
    int antecedencia = definicao->holofote_aviso + ((partida->marcas & MARCA_BENEFICIO_AVISO) ? 2 : 0);
    partida->holofote_chegada = chegada < 1 ? 1 : chegada;
    partida->holofote_aviso_em = partida->holofote_chegada - antecedencia;
    if (partida->holofote_aviso_em < 0) partida->holofote_aviso_em = 0;
    partida->holofote = EMERGENCIA_AGUARDANDO;
}

static void agendar_inspecao(Partida *partida, const DefinicaoNoite *definicao) {
    if (definicao->inspecao_chegada <= 0) return;
    int chegada = definicao->inspecao_chegada;
    if (partida->marcas & MARCA_PROGRAMACAO_FALSA) chegada += 1;
    if (farol_com_programacao_real(partida)) chegada -= 2;
    partida->inspecao_chegada = chegada < 1 ? 1 : chegada;
    partida->inspecao_aviso_em = partida->inspecao_chegada - definicao->inspecao_aviso;
    if (partida->inspecao_aviso_em < 0) partida->inspecao_aviso_em = 0;
    partida->inspecao = EMERGENCIA_AGUARDANDO;
}

static void aplicar_preparativo(Partida *partida) {
    if (partida->preparativo == PREPARO_BATERIA) {
        partida->pontos_maximos += 2;
        partida->pontos_acao += 2;
    }
    if (partida->preparativo == PREPARO_ESCONDERIJO) partida->provas_escondidas = true;
}

static void chegar_mensagens(Partida *partida) {
    int total;
    const Mensagem *mensagens = campanha_mensagens(&total);
    for (int i = 0; i < total; i++) {
        if (mensagens[i].chegada != partida->relogio || !mensagem_desta_noite(partida, &mensagens[i])) continue;
        enfileirar(&partida->fila, mensagens[i].id);
        partida_avisar(partida, SOM_MENSAGEM, "O receptor estala: chegou um sinal de %s.", mensagens[i].remetente);
    }
}

static void atualizar_emergencias(Partida *partida);

bool partida_iniciar_operacao(Partida *partida) {
    partida_limpar_aviso(partida);
    if (partida->fase != FASE_PREPARACAO) return false;
    if (partida->preparativo == PREPARO_NENHUM) {
        partida_avisar(partida, SOM_ERRO, "Antes de ligar a rádio, escolha na bancada como vai se preparar para esta noite.");
        return false;
    }
    partida->fase = FASE_OPERACAO;
    aplicar_preparativo(partida);
    const DefinicaoNoite *definicao = partida_definicao(partida);
    agendar_holofote(partida, definicao);
    agendar_inspecao(partida, definicao);
    char fim[6];
    partida_formatar_hora(partida->pontos_acao, fim);
    partida_avisar(partida, SOM_CONFIRMA, "A rádio está no ar. A bateria aguenta até %s: cada decisão consome tempo.", fim);
    chegar_mensagens(partida);
    atualizar_emergencias(partida);
    return true;
}

/* ---------- emergências ---------- */

static bool holofote_bloqueado(Partida *partida) {
    if (partida->cortina_fechada) return true;
    if (partida->preparativo == PREPARO_CORTINA && !partida->cortina_reforcada_usada) {
        partida->cortina_reforcada_usada = true;
        return true;
    }
    return false;
}

static void resolver_holofote(Partida *partida) {
    partida->holofote = EMERGENCIA_RESOLVIDA;
    if (holofote_bloqueado(partida)) {
        partida_avisar(partida, SOM_HOLOFOTE, "A luz bateu na cortina e seguiu adiante. Ninguém viu nada.");
    } else if (partida->jogador_na_area_holofote) {
        partida->holofote_viu = true;
        partida_somar_suspeita(partida, 25);
        partida_avisar(partida, SOM_HOLOFOTE, "O feixe parou em cima de você por um segundo longo demais. Alguém na torre viu. (suspeita +25)");
    } else {
        partida_somar_suspeita(partida, 5);
        partida_avisar(partida, SOM_HOLOFOTE, "A luz varreu a cabine; você ficou na sombra, mas o quarto aceso chamou atenção. (suspeita +5)");
    }
}

bool partida_resolver_inspecao(Partida *partida) {
    partida->inspecao = EMERGENCIA_RESOLVIDA;
    if (!partida->provas_escondidas) {
        partida_avisar(partida, SOM_INSPECAO, "O primeiro agente vai direto à bancada. Os papéis estavam à vista.");
        return true;
    }
    int risco = partida->suspeita + (partida->equipamentos_ligados ? 30 : 0) +
                (farol_com_programacao_real(partida) ? 15 : 0) - ((partida->marcas & MARCA_PROGRAMACAO_FALSA) ? 25 : 0);
    if (risco >= RISCO_CAPTURA) {
        partida_avisar(partida, SOM_INSPECAO, "Eles não acreditaram em você. Arrancaram as tábuas do piso até achar a rádio.");
        return true;
    }
    partida->inspecao_superada = true;
    partida_avisar(partida, SOM_INSPECAO, "Reviraram gavetas, olharam debaixo do colchão... e foram embora. Por pouco.");
    return false;
}

static void atualizar_holofote(Partida *partida) {
    if (partida->holofote == EMERGENCIA_AGUARDANDO && partida->relogio >= partida->holofote_aviso_em) {
        partida->holofote = EMERGENCIA_AVISADA;
        char hora[6];
        partida_formatar_hora(partida->holofote_chegada, hora);
        partida_avisar(partida, SOM_HOLOFOTE, "Uma sirene ao longe. O holofote da torre vai varrer esta janela às %s.", hora);
    }
    if (partida->holofote == EMERGENCIA_AVISADA && partida->relogio >= partida->holofote_chegada) resolver_holofote(partida);
}

/* Devolve true se a inspeção terminou em captura. */
static bool atualizar_inspecao(Partida *partida) {
    if (partida->inspecao == EMERGENCIA_AGUARDANDO && partida->relogio >= partida->inspecao_aviso_em) {
        partida->inspecao = EMERGENCIA_AVISADA;
        char hora[6];
        partida_formatar_hora(partida->inspecao_chegada, hora);
        partida_avisar(partida, SOM_INSPECAO, "Batidas no andar de baixo. Agentes revistam o prédio e chegam à sua porta às %s.", hora);
    }
    if (partida->inspecao == EMERGENCIA_AVISADA && partida->relogio >= partida->inspecao_chegada) {
        return partida_resolver_inspecao(partida);
    }
    return false;
}

static void atualizar_emergencias(Partida *partida) {
    atualizar_holofote(partida);
    if (atualizar_inspecao(partida)) {
        partida->final = FINAL_CAPTURA;
        partida_encerrar(partida, FIM_CAPTURA);
    }
}

/* ---------- relógio de ações ---------- */

bool partida_pode_gastar(Partida *partida, int custo) {
    if (partida->fase != FASE_OPERACAO) {
        partida_avisar(partida, SOM_ERRO, "A rádio ainda está desligada. Ligue o transmissor para começar a noite.");
        return false;
    }
    if (partida->pontos_acao < custo) {
        partida_avisar(partida, SOM_ERRO, "Não há tempo para isso: precisa de %d min e a bateria só aguenta mais %d min.", custo * MINUTOS_POR_ACAO,
                       partida->pontos_acao * MINUTOS_POR_ACAO);
        return false;
    }
    return true;
}

bool partida_gastar(Partida *partida, int custo) {
    partida->pontos_acao -= custo;
    for (int i = 0; i < custo && partida->fase == FASE_OPERACAO; i++) {
        partida->relogio++;
        chegar_mensagens(partida);
        atualizar_emergencias(partida);
    }
    return partida->fase == FASE_OPERACAO;
}

void partida_verificar_fim(Partida *partida) {
    if (partida->fase == FASE_OPERACAO && partida->pontos_acao <= 0) partida_encerrar(partida, FIM_SEM_PONTOS);
}

/* ---------- encerramento ---------- */

static void aplicar_tarefas_perdidas(Partida *partida) {
    int total;
    const Tarefa *tarefas = campanha_tarefas(&total);
    for (int i = 0; i < total; i++) {
        const Tarefa *tarefa = &tarefas[i];
        if (!partida_tarefa_visivel(partida, tarefa) || !tarefa->obrigatoria) continue;
        if (partida_tarefa_concluida(partida, tarefa)) continue;
        partida_somar_suspeita(partida, tarefa->suspeita_falha);
        adicionar_consequencia(partida, tarefa->falha);
    }
}

static void definir_final(Partida *partida) {
    if (partida->motivo_fim == FIM_CAPTURA) {
        partida->final = FINAL_CAPTURA;
        return;
    }
    if (partida->noite < TOTAL_NOITES) return;
    bool completa = (partida->marcas & MARCA_DENUNCIA_FINAL) && partida_total_provas(partida) >= PROVAS_PARA_COMPLETA;
    partida->final = completa ? FINAL_COMPLETA : FINAL_INCOMPLETA;
}

void partida_encerrar(Partida *partida, MotivoFim motivo) {
    if (partida->fase == FASE_ENCERRADA) return;
    partida->fase = FASE_ENCERRADA;
    partida->motivo_fim = motivo;
    /* Se a noite acaba antes de os agentes chegarem, a inspeção acontece no fim dela. */
    if (motivo != FIM_CAPTURA && (partida->inspecao == EMERGENCIA_AGUARDANDO || partida->inspecao == EMERGENCIA_AVISADA)) {
        if (partida_resolver_inspecao(partida)) partida->motivo_fim = FIM_CAPTURA;
    }
    if (partida->motivo_fim == FIM_RASTREADO) adicionar_consequencia(partida, "A polícia localizou o sinal e você precisou desligar tudo antes da hora.");
    if (partida->motivo_fim == FIM_CAPTURA) adicionar_consequencia(partida, "Os agentes encontraram a rádio.");
    aplicar_tarefas_perdidas(partida);
    definir_final(partida);
}

/* ---------- consultas ---------- */

const DefinicaoNoite *partida_definicao(const Partida *partida) { return campanha_noite(partida->noite); }

const Mensagem *partida_mensagem_da_caixa(const Partida *partida, int indice_caixa) {
    if (indice_caixa < 0 || indice_caixa >= partida->total_caixa) return NULL;
    return campanha_mensagem(partida->caixa[indice_caixa].id_mensagem);
}

bool partida_novidade_visivel(const Partida *partida, const Novidade *novidade) {
    return novidade->noite == partida->noite && marcas_permitem(partida, novidade->exige_marca, novidade->bloqueia_marca);
}

bool partida_documento_na_bancada(const Partida *partida, const Documento *documento) {
    return documento->noite <= partida->noite;
}

bool partida_documento_liberado(const Partida *partida, const Documento *documento) {
    if (!partida_documento_na_bancada(partida, documento)) return false;
    if (!documento->exige_investigacao) return true;
    return documento->id < MAX_DOCUMENTOS && partida->documento_investigado[documento->id];
}

bool partida_tarefa_visivel(const Partida *partida, const Tarefa *tarefa) {
    return tarefa->noite == partida->noite && marcas_permitem(partida, tarefa->exige_marca, tarefa->bloqueia_marca);
}

bool partida_codigo_no_ar(const Partida *partida, const char *codigo) {
    for (int i = 0; i < partida->total_codigos_no_ar; i++) {
        if (strcmp(partida->codigos_no_ar[i], codigo) == 0) return true;
    }
    return false;
}

static bool codigo_foi_transmitido(const Partida *partida, const char *codigo) {
    for (int i = 0; i < partida->total_registros; i++) {
        if (strcmp(partida->registros[i].codigo, codigo) == 0) return true;
    }
    return false;
}

static bool mensagem_desmascarada(const Partida *partida, int id_mensagem) {
    for (int i = 0; i < partida->total_caixa; i++) {
        const EntradaCaixa *entrada = &partida->caixa[i];
        if (entrada->id_mensagem != id_mensagem || entrada->transmitida) continue;
        return entrada->confirmada || entrada->contradicao || entrada->descartada;
    }
    return false;
}

bool partida_tarefa_concluida(const Partida *partida, const Tarefa *tarefa) {
    switch (tarefa->tipo) {
        case META_LER_NOVIDADES: return partida->leu_novidades;
        case META_PREPARATIVO: return partida->preparativo != PREPARO_NENHUM;
        case META_INICIAR: return partida->fase != FASE_PREPARACAO;
        case META_TRANSMITIR: return partida_codigo_no_ar(partida, tarefa->codigo);
        case META_DESMASCARAR: return mensagem_desmascarada(partida, tarefa->valor);
        case META_EVITAR: return !codigo_foi_transmitido(partida, tarefa->codigo);
        /* Se a noite acabou antes de a luz chegar, o jogador também escapou. */
        case META_HOLOFOTE: return !partida->holofote_viu && (partida->holofote == EMERGENCIA_RESOLVIDA || partida->fase == FASE_ENCERRADA);
        case META_INSPECAO: return partida->inspecao_superada;
        case META_RASTREAMENTO_MAXIMO: return partida->fase != FASE_PREPARACAO && partida->rastreamento <= tarefa->valor;
        case META_MARCA: return (partida->marcas & (unsigned)tarefa->valor) != 0;
        case META_INVESTIGAR: return tarefa->valor < MAX_DOCUMENTOS && partida->documento_investigado[tarefa->valor];
        case META_PROVAS: return partida_total_provas(partida) >= tarefa->valor;
        case META_PROVAS_ESCONDIDAS: return partida->provas_escondidas;
    }
    return false;
}

int partida_total_provas(const Partida *partida) {
    int total = 0;
    for (unsigned marca = partida->marcas & MARCAS_PROVAS; marca; marca &= marca - 1) total++;
    return total;
}

int partida_custo_confirmacao(const Partida *partida) { return partida->confirmacao_gratis ? 0 : 1; }

int partida_custo_esconder(const Partida *partida) {
    return (partida->marcas & MARCA_BENEFICIO_ESCONDERIJO) ? 0 : 1;
}

int partida_acoes_ate_holofote(const Partida *partida) {
    if (partida->holofote != EMERGENCIA_AVISADA) return -1;
    return partida->holofote_chegada - partida->relogio;
}

int partida_acoes_ate_inspecao(const Partida *partida) {
    if (partida->inspecao != EMERGENCIA_AVISADA) return -1;
    return partida->inspecao_chegada - partida->relogio;
}

void partida_formatar_hora(int acao, char saida[6]) {
    int minutos = (HORA_INICIAL_EM_MINUTOS + acao * MINUTOS_POR_ACAO) % (24 * 60);
    if (minutos < 0) minutos += 24 * 60;
    int horas = minutos / 60, resto = minutos % 60;
    /* Dígito a dígito: o formato é sempre HH:MM e cabe exatamente em 6 bytes. */
    saida[0] = (char)('0' + horas / 10);
    saida[1] = (char)('0' + horas % 10);
    saida[2] = ':';
    saida[3] = (char)('0' + resto / 10);
    saida[4] = (char)('0' + resto % 10);
    saida[5] = '\0';
}

const Tarefa *partida_proxima_obrigatoria(const Partida *partida) {
    int total;
    const Tarefa *tarefas = campanha_tarefas(&total);
    for (int i = 0; i < total; i++) {
        if (!tarefas[i].obrigatoria || !partida_tarefa_visivel(partida, &tarefas[i])) continue;
        if (!partida_tarefa_concluida(partida, &tarefas[i])) return &tarefas[i];
    }
    return NULL;
}
