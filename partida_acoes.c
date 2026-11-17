/* Ações do jogador: cada função valida, cobra pontos (se houver custo) e explica o resultado em `aviso`. */
#include "partida_interno.h"
#include <stdio.h>
#include <string.h>

#define RASTREAMENTO_POR_TRANSMISSAO 15
#define RASTREAMENTO_EMBARALHADO 8
#define RASTREAMENTO_TROCA_FREQUENCIA 35
#define RASTREAMENTO_MAXIMO 100
#define SUSPEITA_MENSAGEM_FALSA 20
#define SUSPEITA_DENUNCIA_PRECOCE 10
#define SUSPEITA_RASTREADO 20

/* ---------- preparação ---------- */

void partida_ler_novidades(Partida *partida) { partida->leu_novidades = true; }

bool partida_escolher_preparativo(Partida *partida, Preparativo preparativo) {
    partida_limpar_aviso(partida);
    if (partida->fase != FASE_PREPARACAO || preparativo <= PREPARO_NENHUM || preparativo >= PREPARO_TOTAL) {
        partida_avisar(partida, SOM_ERRO, "Agora é tarde para mudar os preparativos: a rádio já está no ar.");
        return false;
    }
    partida->preparativo = preparativo;
    partida_avisar(partida, SOM_CONFIRMA, "Preparativo da noite: %s.", campanha_preparativo(preparativo)->nome);
    return true;
}

/* ---------- receptor ---------- */

bool partida_receber_mensagem(Partida *partida) {
    partida_limpar_aviso(partida);
    int id_mensagem;
    if (partida->total_caixa >= MAX_CAIXA || !desenfileirar(&partida->fila, &id_mensagem)) {
        partida_avisar(partida, SOM_ERRO, "Só estática. Nenhum sinal esperando.");
        return false;
    }
    EntradaCaixa entrada = {.id_mensagem = id_mensagem, .resposta = -1};
    partida->caixa[partida->total_caixa++] = entrada;
    partida_avisar(partida, SOM_MENSAGEM, "Você sintoniza o sinal de %s e anota a mensagem.", campanha_mensagem(id_mensagem)->remetente);
    return true;
}

static void marcar_exposta(Partida *partida, const Mensagem *mensagem) {
    if (mensagem->veracidade == VERACIDADE_FALSA) partida->marcas |= mensagem->marca_ao_expor;
}

bool partida_confirmar(Partida *partida, int indice_caixa) {
    partida_limpar_aviso(partida);
    EntradaCaixa *entrada = partida_entrada(partida, indice_caixa);
    if (!entrada || entrada->confirmada) return false;
    int custo = partida_custo_confirmacao(partida);
    if (!partida_pode_gastar(partida, custo)) return false;
    const Mensagem *mensagem = campanha_mensagem(entrada->id_mensagem);
    entrada->confirmada = true;
    if (custo == 0) partida->confirmacao_gratis = false;
    marcar_exposta(partida, mensagem);
    partida_avisar(partida, SOM_CONFIRMA, "Resposta da rede: %s", mensagem->confirmacao);
    partida_gastar(partida, custo);
    partida_verificar_fim(partida);
    return true;
}

bool partida_aceitar(Partida *partida, int indice_caixa) {
    partida_limpar_aviso(partida);
    EntradaCaixa *entrada = partida_entrada(partida, indice_caixa);
    const Mensagem *mensagem = partida_mensagem_da_caixa(partida, indice_caixa);
    if (!entrada || !mensagem->transmissivel || entrada->aceita || entrada->transmitida) return false;
    if (!inserirTransmissao(&partida->programacao, mensagem->transmissao)) return false;
    entrada->aceita = true;
    entrada->descartada = false;
    partida_avisar(partida, SOM_CONFIRMA, "'%s' entrou no fim do roteiro da noite.", mensagem->transmissao.codigo);
    return true;
}

/* Tira da programação o nó que veio desta mensagem (a busca é pelo código). */
static void tirar_da_programacao(Partida *partida, const Mensagem *mensagem) {
    NoTransmissao *no = buscarTransmissao(&partida->programacao, mensagem->transmissao.codigo);
    if (no && no->dados.id == mensagem->id) removerTransmissao(&partida->programacao, no);
}

bool partida_descartar(Partida *partida, int indice_caixa) {
    partida_limpar_aviso(partida);
    EntradaCaixa *entrada = partida_entrada(partida, indice_caixa);
    if (!entrada || entrada->descartada || entrada->transmitida) return false;
    const Mensagem *mensagem = campanha_mensagem(entrada->id_mensagem);
    if (entrada->aceita) tirar_da_programacao(partida, mensagem);
    entrada->aceita = false;
    entrada->descartada = true;
    partida_avisar(partida, SOM_PAPEL, "Você risca a mensagem de %s do caderno.", mensagem->remetente);
    return true;
}

bool partida_responder(Partida *partida, int indice_caixa, int resposta) {
    partida_limpar_aviso(partida);
    EntradaCaixa *entrada = partida_entrada(partida, indice_caixa);
    const Mensagem *mensagem = partida_mensagem_da_caixa(partida, indice_caixa);
    if (!entrada || entrada->resposta >= 0 || resposta < 0 || resposta >= mensagem->total_respostas) return false;
    const RespostaMensagem *escolhida = &mensagem->respostas[resposta];
    if (escolhida->custo > 0 && !partida_pode_gastar(partida, escolhida->custo)) return false;
    entrada->resposta = resposta;
    partida->marcas |= escolhida->marca;
    partida_avisar(partida, SOM_CONFIRMA, "%s", escolhida->resultado);
    partida_gastar(partida, escolhida->custo);
    partida_verificar_fim(partida);
    return true;
}

/* ---------- bancada ---------- */

bool partida_comparar(Partida *partida, int indice_caixa, int id_documento) {
    partida_limpar_aviso(partida);
    EntradaCaixa *entrada = partida_entrada(partida, indice_caixa);
    const Documento *documento = campanha_documento(id_documento);
    if (!entrada || !documento || !partida_documento_liberado(partida, documento)) return false;
    const Mensagem *mensagem = campanha_mensagem(entrada->id_mensagem);
    if (mensagem->documento_contradiz != id_documento) {
        partida_avisar(partida, SOM_PAPEL, "Você relê a mensagem de %s ao lado de '%s'. Nada parece fora do lugar.", mensagem->remetente, documento->titulo);
        return false;
    }
    entrada->contradicao = true;
    marcar_exposta(partida, mensagem);
    partida_avisar(partida, SOM_PAPEL, "Algo não fecha na mensagem de %s. %s", mensagem->remetente, mensagem->contradicao);
    return true;
}

bool partida_investigar(Partida *partida, int id_documento) {
    partida_limpar_aviso(partida);
    const Documento *documento = campanha_documento(id_documento);
    if (!documento || !documento->exige_investigacao || documento->id >= MAX_DOCUMENTOS) return false;
    if (partida->documento_investigado[documento->id] || !partida_documento_na_bancada(partida, documento)) return false;
    if (!partida_pode_gastar(partida, 1)) return false;
    partida->documento_investigado[documento->id] = true;
    partida_avisar(partida, SOM_PAPEL, "Depois de muito procurar, você conseguiu: '%s' agora está na bancada.", documento->titulo);
    partida_gastar(partida, 1);
    partida_verificar_fim(partida);
    return true;
}

bool partida_esconder_provas(Partida *partida) {
    partida_limpar_aviso(partida);
    if (partida->provas_escondidas) return false;
    int custo = partida_custo_esconder(partida);
    if (!partida_pode_gastar(partida, custo)) return false;
    partida->provas_escondidas = true;
    partida_avisar(partida, SOM_PAPEL, "Os papéis somem sob a tábua solta do piso. A bancada parece a de um estudante.");
    partida_gastar(partida, custo);
    partida_verificar_fim(partida);
    return true;
}

/* ---------- transmissor ---------- */

static void marcar_aceita_como(Partida *partida, int id_mensagem, bool aceita, bool transmitida) {
    for (int i = 0; i < partida->total_caixa; i++) {
        if (partida->caixa[i].id_mensagem != id_mensagem) continue;
        partida->caixa[i].aceita = aceita;
        partida->caixa[i].transmitida = transmitida;
    }
}

bool partida_remover_transmissao(Partida *partida, NoTransmissao *no) {
    partida_limpar_aviso(partida);
    if (!no) return false;
    int id = no->dados.id;
    char codigo[TEXTO_CODIGO];
    snprintf(codigo, sizeof(codigo), "%s", no->dados.codigo);
    removerTransmissao(&partida->programacao, no);
    marcar_aceita_como(partida, id, false, false);
    partida_avisar(partida, SOM_PAPEL, "'%s' saiu do roteiro. A mensagem continua anotada no receptor.", codigo);
    return true;
}

bool partida_mover_transmissao(Partida *partida, NoTransmissao *no, int direcao) {
    partida_limpar_aviso(partida);
    if (!moverTransmissao(&partida->programacao, no, direcao)) return false;
    partida_avisar(partida, SOM_CONFIRMA, direcao < 0 ? "Você adianta essa transmissão no roteiro." : "Você deixa essa transmissão para mais tarde.");
    return true;
}

void partida_priorizar(Partida *partida) {
    partida_limpar_aviso(partida);
    ordenarPorPrazo(&partida->programacao);
    partida_avisar(partida, SOM_CONFIRMA, "Roteiro reorganizado: o que tem menos tempo vai primeiro. Confira se cada aviso ainda vem depois da sua senha.");
}

/* Decide o resultado de uma transmissão que acabou de ir ao ar. */
static bool avaliar_transmissao(Partida *partida, const Transmissao *transmissao, const Mensagem *mensagem, char *resultado) {
    if (partida->relogio > transmissao->prazo) {
        char limite[6], agora[6];
        partida_formatar_hora(transmissao->prazo, limite);
        partida_formatar_hora(partida->relogio, agora);
        snprintf(resultado, TEXTO_AVISO, "Tarde demais: só servia até %s, e foi ao ar às %s.", limite, agora);
        return false;
    }
    if (transmissao->requer[0] && !partida_codigo_no_ar(partida, transmissao->requer)) {
        snprintf(resultado, TEXTO_AVISO, "Ninguém confiou no aviso: a senha %s não tinha ido ao ar antes.", transmissao->requer);
        return false;
    }
    if (mensagem->veracidade == VERACIDADE_FALSA) {
        partida_somar_suspeita(partida, SUSPEITA_MENSAGEM_FALSA);
        snprintf(resultado, TEXTO_AVISO, "Era uma armadilha. %s (suspeita +%d)", mensagem->resultado, SUSPEITA_MENSAGEM_FALSA);
        return false;
    }
    partida->marcas |= transmissao->consequencia;
    if (mensagem->veracidade == VERACIDADE_INCOMPLETA) {
        partida_somar_suspeita(partida, SUSPEITA_DENUNCIA_PRECOCE);
        snprintf(resultado, TEXTO_AVISO, "Cedo demais. %s (suspeita +%d)", mensagem->resultado, SUSPEITA_DENUNCIA_PRECOCE);
        return false;
    }
    partida_registrar_codigo(partida, transmissao->codigo);
    snprintf(resultado, TEXTO_AVISO, "%s", mensagem->resultado);
    return true;
}

static void registrar_transmissao(Partida *partida, const Transmissao *transmissao, bool sucesso, const char *resultado) {
    if (partida->total_registros >= MAX_REGISTROS) return;
    RegistroTransmissao *registro = &partida->registros[partida->total_registros++];
    snprintf(registro->codigo, sizeof(registro->codigo), "%s", transmissao->codigo);
    snprintf(registro->titulo, sizeof(registro->titulo), "%s", transmissao->titulo);
    snprintf(registro->resultado, sizeof(registro->resultado), "%s", resultado);
    registro->sucesso = sucesso;
}

static void subir_rastreamento(Partida *partida) {
    partida->rastreamento += partida->preparativo == PREPARO_EMBARALHADOR ? RASTREAMENTO_EMBARALHADO : RASTREAMENTO_POR_TRANSMISSAO;
    if (partida->rastreamento < RASTREAMENTO_MAXIMO) return;
    partida->rastreamento = RASTREAMENTO_MAXIMO;
    partida_somar_suspeita(partida, SUSPEITA_RASTREADO);
    partida_avisar(partida, SOM_ALERTA, "Um carro de escuta parou na esquina: acharam o sinal. Você arranca os cabos e a noite acaba aqui.");
    partida_encerrar(partida, FIM_RASTREADO);
}

bool partida_transmitir_proxima(Partida *partida) {
    partida_limpar_aviso(partida);
    NoTransmissao *primeiro = partida->programacao.inicio;
    if (!primeiro) {
        partida_avisar(partida, SOM_ERRO, "O roteiro está vazio. Aceite mensagens no receptor primeiro.");
        return false;
    }
    if (!partida->equipamentos_ligados) {
        partida_avisar(partida, SOM_ERRO, "Os equipamentos estão desligados. Ligue-os antes de transmitir.");
        return false;
    }
    if (!partida_pode_gastar(partida, primeiro->dados.custo)) return false;
    Transmissao transmissao;
    executarProximaTransmissao(&partida->programacao, &transmissao);
    marcar_aceita_como(partida, transmissao.id, false, true);
    if (!partida_gastar(partida, transmissao.custo)) return true; /* capturado durante a transmissão */
    char resultado[TEXTO_AVISO];
    bool sucesso = avaliar_transmissao(partida, &transmissao, campanha_mensagem(transmissao.id), resultado);
    registrar_transmissao(partida, &transmissao, sucesso, resultado);
    partida_avisar(partida, sucesso ? SOM_TRANSMISSAO : SOM_FALHA, "%s: %s", transmissao.codigo, resultado);
    subir_rastreamento(partida);
    partida_verificar_fim(partida);
    return true;
}

/* ---------- frequência, janela, equipamentos e porta ---------- */

bool partida_trocar_frequencia(Partida *partida) {
    partida_limpar_aviso(partida);
    if (!partida_pode_gastar(partida, 1)) return false;
    partida->rastreamento -= RASTREAMENTO_TROCA_FREQUENCIA;
    if (partida->rastreamento < 0) partida->rastreamento = 0;
    partida_avisar(partida, SOM_CONFIRMA, "Você troca de frequência no meio da estática. Rastreamento em %d%%.", partida->rastreamento);
    partida_gastar(partida, 1);
    partida_verificar_fim(partida);
    return true;
}

bool partida_alternar_cortina(Partida *partida) {
    partida_limpar_aviso(partida);
    if (!partida_pode_gastar(partida, 1)) return false;
    partida->cortina_fechada = !partida->cortina_fechada;
    partida_avisar(partida, SOM_CONFIRMA, partida->cortina_fechada ? "Você puxa a cortina pesada. A rua some." : "Você abre a cortina. A rua volta a olhar para dentro.");
    partida_gastar(partida, 1);
    partida_verificar_fim(partida);
    return true;
}

bool partida_alternar_equipamentos(Partida *partida) {
    partida_limpar_aviso(partida);
    if (!partida->equipamentos_ligados) {
        partida->equipamentos_ligados = true;
        partida_avisar(partida, SOM_CONFIRMA, "As válvulas esquentam de novo. A rádio volta a respirar.");
        return true;
    }
    if (!partida_pode_gastar(partida, 1)) return false;
    partida->equipamentos_ligados = false;
    partida_avisar(partida, SOM_CONFIRMA, "Você desliga tudo e cobre o transmissor com um lençol. Parece só um quarto.");
    partida_gastar(partida, 1);
    partida_verificar_fim(partida);
    return true;
}

bool partida_responder_porta(Partida *partida) {
    partida_limpar_aviso(partida);
    if (partida->fase != FASE_OPERACAO || partida->inspecao != EMERGENCIA_AVISADA) return false;
    if (partida_resolver_inspecao(partida)) {
        partida->final = FINAL_CAPTURA;
        partida_encerrar(partida, FIM_CAPTURA);
    }
    return true;
}

bool partida_dormir(Partida *partida) {
    partida_limpar_aviso(partida);
    if (partida->fase != FASE_OPERACAO) return false;
    partida_encerrar(partida, FIM_DORMIU);
    return true;
}
