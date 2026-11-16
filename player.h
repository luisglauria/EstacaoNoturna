#ifndef PLAYER_H
#define PLAYER_H

#include "raylib.h"

typedef struct { Vector2 position; float speed; } Player;

void player_init(Player *player, Vector2 position);
void player_update(Player *player, float delta, Rectangle bounds);
Rectangle player_hitbox(const Player *player);
void player_draw(const Player *player);

#endif
