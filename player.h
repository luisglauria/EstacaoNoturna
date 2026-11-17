#ifndef PLAYER_H
#define PLAYER_H

#include "raylib.h"
#include <stdbool.h>

typedef struct {
    Vector2 position;
    Vector2 facing;   /* última direção andada, para desenhar o rosto do lado certo */
    float speed;
    float walk_time;  /* acumula só enquanto anda; move pernas e balanço */
    bool moving;
} Player;

/* Pergunta ao cenário se uma hitbox colide; vem por parâmetro para o player não depender da cabine. */
typedef bool (*BlockedCheck)(Rectangle hitbox);

void player_init(Player *player, Vector2 position);
void player_update(Player *player, float delta, BlockedCheck is_blocked);
Rectangle player_hitbox_at(Vector2 position);
/* Linha de pixel do mundo onde ficam os pés (usada para ordenar quem fica na frente). */
int player_feet_pixel(const Player *player);
/* Desenha em pixels do mundo (dentro de pixel_mundo_comecar/terminar). */
void player_draw(const Player *player);
/* Verdadeiro no quadro em que um pé toca o chão (som de passo). */
bool player_step_landed(const Player *player, float delta);

#endif
