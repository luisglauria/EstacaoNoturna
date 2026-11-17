#include "interface.h"
#include <math.h>
#include <stdlib.h>

/* Música e efeitos sintetizados em memória, sem arquivos de áudio. */
#define TAXA 44100
#define DURACAO_MUSICA 16.0f
#define VELOCIDADE_TENSAO 0.35f

static Sound efeitos[EFEITO_TOTAL];
static Sound musica_base;
static Sound musica_tensao;
static bool audio_pronto = false;
static float volume_efeitos = 0.8f;
static float tensao_alvo = 0.0f;
static float tensao_atual = 0.0f;

typedef struct {
    float *amostras;
    int quadros;
} Buffer;

static Buffer criar_buffer(float segundos) {
    Buffer buffer = {.quadros = (int)(segundos * TAXA)};
    buffer.amostras = calloc((size_t)buffer.quadros, sizeof(float));
    return buffer;
}

static float envelope(float t, float duracao, float ataque) {
    if (t < ataque) return t / ataque;
    float resto = 1.0f - (t - ataque) / (duracao - ataque);
    return resto * resto;
}

static void somar_tom(Buffer *b, float inicio, float duracao, float frequencia, float volume) {
    int primeiro = (int)(inicio * TAXA), total = (int)(duracao * TAXA);
    for (int i = 0; i < total && primeiro + i < b->quadros; i++) {
        float t = (float)i / TAXA;
        b->amostras[primeiro + i] += sinf(2 * PI * frequencia * t) * volume * envelope(t, duracao, 0.004f);
    }
}

static void somar_varredura(Buffer *b, float inicio, float duracao, float de, float ate, float volume) {
    int primeiro = (int)(inicio * TAXA), total = (int)(duracao * TAXA);
    float fase = 0;
    for (int i = 0; i < total && primeiro + i < b->quadros; i++) {
        float t = (float)i / TAXA;
        fase += 2 * PI * (de + (ate - de) * t / duracao) / TAXA;
        b->amostras[primeiro + i] += sinf(fase) * volume * envelope(t, duracao, duracao * 0.2f);
    }
}

static void somar_ruido(Buffer *b, float inicio, float duracao, float volume, float suavidade, float ataque) {
    int primeiro = (int)(inicio * TAXA), total = (int)(duracao * TAXA);
    float filtrado = 0;
    for (int i = 0; i < total && primeiro + i < b->quadros; i++) {
        float branco = (float)rand() / (float)RAND_MAX * 2.0f - 1.0f;
        filtrado = filtrado * suavidade + branco * (1.0f - suavidade);
        float t = (float)i / TAXA;
        b->amostras[primeiro + i] += filtrado * volume * envelope(t, duracao, ataque);
    }
}

static void somar_eco(Buffer *b, float atraso, float ganho) {
    int deslocamento = (int)(atraso * TAXA);
    for (int i = deslocamento; i < b->quadros; i++) b->amostras[i] += b->amostras[i - deslocamento] * ganho;
}

static Sound para_som(Buffer *b) {
    short *dados = malloc((size_t)b->quadros * sizeof(short));
    for (int i = 0; i < b->quadros; i++) dados[i] = (short)(fmaxf(-1.0f, fminf(1.0f, b->amostras[i])) * 32000);
    Wave onda = {.frameCount = (unsigned)b->quadros, .sampleRate = TAXA, .sampleSize = 16, .channels = 1, .data = dados};
    Sound som = LoadSoundFromWave(onda);
    free(dados);
    free(b->amostras);
    return som;
}

static void compor_base(Buffer *b) {
    const float acorde[] = {110.0f, 130.8125f, 164.8125f, 174.625f};
    for (int i = 0; i < b->quadros; i++) {
        float t = (float)i / TAXA;
        float drone = sinf(2 * PI * 55.0f * t) * 0.16f + sinf(2 * PI * 55.5f * t) * 0.12f;
        float pad = 0;
        for (int nota = 0; nota < 4; nota++) {
            float respiracao = 0.5f - 0.5f * cosf(2 * PI * (t / DURACAO_MUSICA + nota * 0.25f));
            pad += sinf(2 * PI * acorde[nota] * t) * 0.035f * respiracao;
        }
        float agudo = sinf(2 * PI * 659.25f * t) * 0.012f * powf(sinf(PI * t / DURACAO_MUSICA), 6);
        b->amostras[i] = drone + pad + agudo;
    }
    somar_ruido(b, 0, DURACAO_MUSICA, 0.02f, 0.93f, 2.0f);
    somar_tom(b, 3.0f, 0.06f, 1180, 0.03f);
    somar_tom(b, 3.15f, 0.18f, 1180, 0.03f);
    somar_tom(b, 11.4f, 0.06f, 1180, 0.025f);
    somar_varredura(b, 8.0f, 1.4f, 92, 68, 0.05f);
    somar_ruido(b, 8.0f, 1.4f, 0.02f, 0.97f, 0.3f);
    somar_eco(b, 0.33f, 0.35f);
}

static void compor_tensao(Buffer *b) {
    for (int i = 0; i < b->quadros; i++) {
        float t = (float)i / TAXA;
        float cordas = (sinf(2 * PI * 880.0f * t) + sinf(2 * PI * 886.0f * t)) * 0.018f * (0.6f + 0.4f * sinf(2 * PI * 8.0f * t));
        float graves = sinf(2 * PI * 110.0f * t) * 0.05f * (fmodf(t, 0.5f) < 0.12f ? 1.0f - fmodf(t, 0.5f) / 0.12f : 0.0f);
        b->amostras[i] = cordas + graves;
    }
    for (int batida = 0; batida < 16; batida++) {
        somar_tom(b, batida * 1.0f, 0.14f, 48, 0.32f);
        somar_tom(b, batida * 1.0f + 0.2f, 0.12f, 44, 0.22f);
    }
}

static Sound gerar_efeito(EfeitoSonoro efeito) {
    Buffer b = criar_buffer(efeito == EFEITO_TROVAO ? 3.2f : 1.8f);
    switch (efeito) {
        case EFEITO_CLIQUE: somar_tom(&b, 0, 0.025f, 900, 0.12f); break;
        case EFEITO_CONFIRMA: somar_tom(&b, 0, 0.08f, 440, 0.2f); somar_tom(&b, 0.08f, 0.12f, 659, 0.18f); somar_eco(&b, 0.12f, 0.3f); break;
        case EFEITO_ERRO: somar_tom(&b, 0, 0.25f, 110, 0.35f); somar_tom(&b, 0, 0.25f, 116, 0.25f); break;
        case EFEITO_FALHA: somar_varredura(&b, 0, 0.7f, 400, 120, 0.3f); somar_ruido(&b, 0, 0.7f, 0.1f, 0.6f, 0.05f); break;
        case EFEITO_MENSAGEM:
            somar_ruido(&b, 0, 0.35f, 0.16f, 0.25f, 0.02f);
            for (int tecla = 0; tecla < 6; tecla++) somar_ruido(&b, 0.38f + tecla * 0.07f, 0.02f, 0.2f, 0.1f, 0.002f);
            break;
        case EFEITO_SIRENE: somar_varredura(&b, 0, 0.8f, 480, 690, 0.16f); somar_varredura(&b, 0.8f, 0.8f, 690, 480, 0.16f); somar_eco(&b, 0.21f, 0.5f); break;
        case EFEITO_BATIDAS:
            for (int batida = 0; batida < 3; batida++) {
                somar_tom(&b, batida * 0.3f, 0.07f, 130, 0.55f);
                somar_ruido(&b, batida * 0.3f, 0.06f, 0.35f, 0.6f, 0.002f);
            }
            somar_eco(&b, 0.09f, 0.35f);
            break;
        case EFEITO_PASSO: somar_ruido(&b, 0, 0.06f, 0.3f, 0.85f, 0.003f); somar_tom(&b, 0, 0.05f, 85, 0.15f); break;
        case EFEITO_TROVAO: somar_ruido(&b, 0, 3.0f, 0.9f, 0.985f, 0.15f); somar_tom(&b, 0.05f, 2.5f, 38, 0.2f); break;
        default: somar_ruido(&b, 0, 0.18f, 0.22f, 0.8f, 0.02f); break;
    }
    return para_som(&b);
}

void audio_iniciar(void) {
    InitAudioDevice();
    audio_pronto = IsAudioDeviceReady();
    if (!audio_pronto) return;
    for (int efeito = 0; efeito < EFEITO_TOTAL; efeito++) efeitos[efeito] = gerar_efeito((EfeitoSonoro)efeito);
    Buffer base = criar_buffer(DURACAO_MUSICA), tensao = criar_buffer(DURACAO_MUSICA);
    compor_base(&base);
    compor_tensao(&tensao);
    musica_base = para_som(&base);
    musica_tensao = para_som(&tensao);
}

void audio_encerrar(void) {
    if (!audio_pronto) return;
    for (int efeito = 0; efeito < EFEITO_TOTAL; efeito++) UnloadSound(efeitos[efeito]);
    UnloadSound(musica_base);
    UnloadSound(musica_tensao);
    CloseAudioDevice();
    audio_pronto = false;
}

void audio_definir_tensao(float tensao) { tensao_alvo = fmaxf(0.0f, fminf(1.0f, tensao)); }

void audio_atualizar(const Configuracoes *configuracoes, float delta) {
    if (!audio_pronto) return;
    float passo = VELOCIDADE_TENSAO * delta;
    tensao_atual += fmaxf(-passo, fminf(passo, tensao_alvo - tensao_atual));
    float geral = configuracoes->volume_geral / 100.0f;
    float musica = geral * configuracoes->volume_musica / 100.0f;
    volume_efeitos = geral * configuracoes->volume_efeitos / 100.0f;
    SetSoundVolume(musica_base, musica * (1.0f - 0.35f * tensao_atual));
    SetSoundVolume(musica_tensao, musica * tensao_atual);
    if (!IsSoundPlaying(musica_base)) {
        PlaySound(musica_base);
        PlaySound(musica_tensao);
    }
}

void audio_tocar(EfeitoSonoro efeito) {
    if (!audio_pronto || efeito < 0 || efeito >= EFEITO_TOTAL) return;
    float ajuste = efeito == EFEITO_PASSO ? 0.35f : 1.0f;
    SetSoundVolume(efeitos[efeito], volume_efeitos * ajuste);
    PlaySound(efeitos[efeito]);
}

void audio_tocar_clique(void) { audio_tocar(EFEITO_CLIQUE); }
