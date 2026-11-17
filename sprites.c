#include "sprites.h"
#include "raylib.h"
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#define LINHAS_TRONCO 18
#define LINHAS_PERNAS 6
#define QUADROS 3 /* parado, passo A, passo B */

/* Paleta: uma letra por cor. '.' é transparente. */
static Color cor_da_letra(char letra) {
    switch (letra) {
        case 'K': return (Color){18, 14, 22, 255};    /* contorno */
        case 'H': return (Color){58, 40, 34, 255};    /* cabelo */
        case 'h': return (Color){92, 64, 48, 255};
        case 'S': return (Color){226, 176, 134, 255}; /* pele */
        case 's': return (Color){178, 128, 98, 255};
        case 'E': return (Color){24, 18, 26, 255};    /* olhos */
        case 'F': return (Color){34, 34, 42, 255};    /* fones */
        case 'f': return (Color){96, 96, 110, 255};
        case 'R': return (Color){160, 44, 50, 255};   /* cachecol */
        case 'r': return (Color){112, 30, 38, 255};
        case 'C': return (Color){62, 74, 100, 255};   /* casaco */
        case 'c': return (Color){40, 48, 68, 255};
        case 'L': return (Color){92, 106, 140, 255};
        case 'P': return (Color){44, 44, 58, 255};    /* calça */
        case 'p': return (Color){28, 28, 38, 255};
        case 'B': return (Color){44, 32, 28, 255};    /* botas */
        case 'b': return (Color){78, 58, 46, 255};
        case 'W': return (Color){150, 110, 70, 255};  /* moldura */
        case 'A': return (Color){200, 180, 142, 255}; /* foto sépia */
        case 'a': return (Color){116, 96, 76, 255};
        case 'M': return (Color){120, 76, 44, 255};   /* madeira do rádio */
        case 'm': return (Color){82, 52, 32, 255};
        case 'G': return (Color){62, 50, 42, 255};    /* grade do alto-falante */
        case 'g': return (Color){38, 30, 26, 255};
        case 'O': return (Color){214, 182, 110, 255}; /* mostrador */
        case 'o': return (Color){150, 118, 70, 255};
        default: return BLANK;
    }
}

/* ---------- operador ---------- */

static const char *const TRONCO_FRENTE[LINHAS_TRONCO] = {
    "......KKKK......", "....KKFFFFKK....", "...KFHHHHHHFK...", "..KFHHhhhhHHFK..", "..KFHHHHHHHHFK..",
    ".KffKHSSSSHKffK.", ".KffKSESSESKffK.", ".KFFKSSSSSSKFFK.", "..KK.KSssSK.KK..", "....KRRRRRRK....",
    "...KCRrRRrRCK...", "..KCLCRRRRCLCK..", ".KCLCCCCCCCCLCK.", ".KCLCCCKKCCCLCK.", ".KcLCCCKKCCCLcK.",
    ".KSKcCCKKCCcKSK.", ".KKKccccccccKKK.", "...KPPPPPPPPK...",
};

static const char *const TRONCO_COSTAS[LINHAS_TRONCO] = {
    "......KKKK......", "....KKFFFFKK....", "...KFHHHHHHFK...", "..KFHHHHHHHHFK..", "..KFHHhHHhHHFK..",
    ".KffKHHHHHHKffK.", ".KffKHHhhHHKffK.", ".KFFKHHHHHHKFFK.", "..KK.KsSSsK.KK..", "....KRRRRRRK....",
    "...KCRRRRRRCK...", "..KCLCCCCCCLCK..", ".KCLCCCCCCCCLCK.", ".KCLCCCcCCCCLCK.", ".KcLCCCcCCCCLcK.",
    ".KSKcCCcCCCcKSK.", ".KKKccccccccKKK.", "...KPPPPPPPPK...",
};

static const char *const TRONCO_LADO[LINHAS_TRONCO] = {
    ".....KKKKK......", "....KHHHHFK.....", "...KHHhhHFK.....", "..KHHHHHHFFK....", "..KSHHHHHffK....",
    ".KSSSHHHHffK....", ".KESSSHHHFFK....", "..KSSSSHHHK.....", "...KKsSSKK......", "....KRRRRK......",
    "...KRRrCCCK.....", "...KCCCCLCK.....", "...KCCCCLCK.....", "...KcCCCLCK.....", "...KcCSSLCK.....",
    "...KcCSScCK.....", "...KKcccccK.....", "....KPPPPK......",
};

static const char *const PERNAS_FRENTE[QUADROS][LINHAS_PERNAS] = {
    {"...KPPPKKPPPK...", "...KPPPKKPPPK...", "...KpPPKKPPpK...", "...KPPK..KPPK...", "..KBBbK..KbBBK..", "..KKKKK..KKKKK.."},
    {"...KPPPKKPPPK...", "...KpPPKKPPPK...", "...KBBbKKPPpK...", "...KKKKK.KPPK...", ".........KbBBK..", ".........KKKKK.."},
    {"...KPPPKKPPPK...", "...KPPPKKPPpK...", "...KpPPKKbBBK...", "...KPPK.KKKKK...", "..KBBbK.........", "..KKKKK........."},
};

static const char *const PERNAS_LADO[QUADROS][LINHAS_PERNAS] = {
    {"....KPPPPK......", "....KPPPpK......", "....KPPPpK......", "....KPPPPK......", "...KBBBBbK......", "...KKKKKKK......"},
    {"....KPPPPK......", "...KPPKKPPK.....", "..KPPK..KPPK....", "..KPK....KPK....", ".KBBK....KBbK...", ".KKKK....KKKK..."},
    {"....KPPPPK......", "....KPPPPK......", "...KPPKKPPK.....", "...KPK..KPK.....", "..KBbK..KBBK....", "..KKKK..KKKK...."},
};

static Texture2D operador[3][QUADROS]; /* [frente, costas, lado][quadro] */

static void pintar_linhas(Image *imagem, const char *const linhas[], int total, int y_inicial) {
    for (int linha = 0; linha < total; linha++) {
        int largura = (int)strlen(linhas[linha]);
        for (int coluna = 0; coluna < largura; coluna++) {
            Color cor = cor_da_letra(linhas[linha][coluna]);
            if (cor.a) ImageDrawPixel(imagem, coluna, y_inicial + linha, cor);
        }
    }
}

static Texture2D montar_operador(const char *const tronco[], const char *const pernas[]) {
    Image imagem = GenImageColor(OPERADOR_LARGURA, OPERADOR_ALTURA, BLANK);
    pintar_linhas(&imagem, tronco, LINHAS_TRONCO, 0);
    pintar_linhas(&imagem, pernas, LINHAS_PERNAS, LINHAS_TRONCO);
    Texture2D textura = LoadTextureFromImage(imagem);
    UnloadImage(imagem);
    return textura;
}

/* ---------- objetos pequenos ---------- */

static const char *const FOTO[] = {
    "KKKKKKKK", "KWWWWWWK", "KWAAAAWK", "KWAaaAWK", "KWAaaAWK", "KWAAaAWK", "KWWWWWWK", "KKKKKKKK",
};

static const char *const RADIO_DOMESTICO[] = {
    "..KKKKKKKK..", ".KMMMMMMMMK.", "KMmGGGGmOOMK", "KMmGgGgmOoMK", "KMmGGGGmOOMK", "KMmmmmmmmmMK", "KMMMMMMMMMMK", ".KKKKKKKKKK.",
};

static Texture2D objetos[SPRITE_TOTAL];

static Texture2D montar_mapa(const char *const linhas[], int total) {
    Image imagem = GenImageColor((int)strlen(linhas[0]), total, BLANK);
    pintar_linhas(&imagem, linhas, total, 0);
    Texture2D textura = LoadTextureFromImage(imagem);
    UnloadImage(imagem);
    return textura;
}

void sprites_iniciar(void) {
    for (int quadro = 0; quadro < QUADROS; quadro++) {
        operador[0][quadro] = montar_operador(TRONCO_FRENTE, PERNAS_FRENTE[quadro]);
        operador[1][quadro] = montar_operador(TRONCO_COSTAS, PERNAS_FRENTE[quadro]);
        operador[2][quadro] = montar_operador(TRONCO_LADO, PERNAS_LADO[quadro]);
    }
    objetos[SPRITE_FOTO] = montar_mapa(FOTO, 8);
    objetos[SPRITE_RADIO_DOMESTICO] = montar_mapa(RADIO_DOMESTICO, 8);
}

void sprites_encerrar(void) {
    for (int vista = 0; vista < 3; vista++) {
        for (int quadro = 0; quadro < QUADROS; quadro++) UnloadTexture(operador[vista][quadro]);
    }
    for (int i = 0; i < SPRITE_TOTAL; i++) UnloadTexture(objetos[i]);
}

/* Ciclo de passos: parado, A, parado, B (no perfil, B repete o passo largo de A). */
static int quadro_do_passo(int passo, bool lado) {
    if (passo == 1) return 1;
    if (passo == 3) return lado ? 1 : 2;
    return passo == 2 && lado ? 2 : 0;
}

void sprite_operador(int x, int y_pes, Direcao direcao, int passo) {
    bool lado = direcao == DIRECAO_ESQUERDA || direcao == DIRECAO_DIREITA;
    int vista = direcao == DIRECAO_BAIXO ? 0 : direcao == DIRECAO_CIMA ? 1 : 2;
    Texture2D textura = operador[vista][quadro_do_passo(passo, lado)];
    float largura = direcao == DIRECAO_DIREITA ? -OPERADOR_LARGURA : OPERADOR_LARGURA; /* direita = perfil espelhado */
    Rectangle fonte = {0, 0, largura, OPERADOR_ALTURA};
    int deslocamento = lado ? 7 : 8;
    DrawTextureRec(textura, fonte, (Vector2){(float)(x - deslocamento), (float)(y_pes - OPERADOR_ALTURA)}, WHITE);
}

void sprite_objeto(SpriteObjeto objeto, int x, int y) {
    if (objeto < 0 || objeto >= SPRITE_TOTAL) return;
    DrawTexture(objetos[objeto], x, y, WHITE);
}
