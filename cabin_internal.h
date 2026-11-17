/* Peças compartilhadas entre cabin.c (geometria, ambiente, luz) e cabin_props.c (móveis e objetos). */
#ifndef CABIN_INTERNAL_H
#define CABIN_INTERNAL_H

#include "cabin.h"
#include "pixel.h"

/* Retângulo em pixels do mundo (a geometria da cabine é pensada nessa grade). */
typedef struct {
    int x, y, w, h;
} PxRect;

typedef struct {
    Equipment id;
    PxRect body;
    const char *label;
    bool solid;        /* bloqueia o jogador (móveis); objetos de parede não bloqueiam */
} EquipmentSpot;

/* Cores base da cabine, antes da luz (a escuridão vem do mapa de luz). */
#define COR_CONTORNO ((Color){14, 11, 16, 255})
#define COR_MADEIRA ((Color){104, 74, 52, 255})
#define COR_MADEIRA_FRENTE ((Color){68, 46, 34, 255})
#define COR_PAPEL ((Color){206, 196, 170, 255})
#define COR_TELA_VERDE ((Color){120, 236, 170, 255})
#define COR_AMBAR ((Color){244, 190, 96, 255})
#define COR_VERMELHA ((Color){236, 76, 70, 255})

const EquipmentSpot *cabin_spots(int *count);
const EquipmentSpot *cabin_spot(Equipment id);
/* Desenha o objeto do ponto interativo (receptor, bancada...) na passada da cena. */
void cabin_draw_spot(const EquipmentSpot *spot, float time, const CabinScene *scene);
/* Partes que brilham do objeto (telas, LEDs, lâmpada). */
void cabin_draw_spot_glow(const EquipmentSpot *spot, float time, const CabinScene *scene);
void cabin_draw_chair(void);
void cabin_draw_rug(void);

#endif
