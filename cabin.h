#ifndef CABIN_H
#define CABIN_H

#include "raylib.h"
#include <stdbool.h>

typedef enum {
    EQUIPMENT_NONE,
    EQUIPMENT_RECEIVER,
    EQUIPMENT_DESK,
    EQUIPMENT_TRANSMITTER,
    EQUIPMENT_FREQUENCY,
    EQUIPMENT_WINDOW,
    EQUIPMENT_DOOR,
    EQUIPMENT_MATTRESS,
    EQUIPMENT_NOTE,       /* arquivo de aço com o bilhete na gaveta */
    EQUIPMENT_PHOTO,
    EQUIPMENT_HOME_RADIO,
    EQUIPMENT_WALL_MARKS,
    EQUIPMENT_COUNT
} Equipment;

/* Estado visual que a cabine precisa saber das regras (cortina, holofote, inspeção...). */
typedef struct {
    bool curtain_closed;
    bool equipment_on;
    bool evidence_hidden;
    bool beam_warning;     /* holofote avisado: o feixe já varre a rua e a área prevista aparece */
    float beam_flash;      /* 0 a 1: clarão no momento em que o holofote passa */
    Rectangle beam_area;   /* em coordenadas da tela virtual */
    bool agents_at_door;
    float lightning;       /* 0 a 1: relâmpago lá fora */
} CabinScene;

/* Desenha a parte fixa da cabine (piso, paredes) uma vez numa textura; chamar depois de pixel_iniciar. */
void cabin_iniciar(void);
void cabin_encerrar(void);

/* Área andável da cabine (o piso), em coordenadas da tela virtual. */
Rectangle cabin_floor(void);
/* Verdadeiro se a hitbox sai do piso ou encosta em algum móvel sólido. */
bool cabin_is_blocked(Rectangle hitbox);
/* Objeto interativo mais próximo ao alcance do jogador, ou EQUIPMENT_NONE. */
Equipment cabin_near_equipment(Vector2 player_position);
const char *cabin_equipment_label(Equipment equipment);

/* Luzes da cena; chamar entre pixel_luz_comecar e pixel_luz_terminar. */
void cabin_draw_lights(float time, const CabinScene *scene);
/* Cena em pixels do mundo, com o jogador desenhado na profundidade certa (callback). */
void cabin_draw(float time, const CabinScene *scene, int player_feet, void (*draw_player)(const void *), const void *player);
/* O que brilha por conta própria (telas, lâmpadas, céu, destaque); chamar depois de pixel_aplicar_luz. */
void cabin_draw_emissive(float time, Equipment highlighted, const CabinScene *scene);

#endif
