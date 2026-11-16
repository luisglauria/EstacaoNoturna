#ifndef GAME_H
#define GAME_H

#include "cabin.h"
#include "player.h"
#include "radio_list.h"
#include "ui.h"

typedef struct {
    Player player;
    RadioList transmissions;
    RadioNode *selected;
    Panel panel;
    Equipment nearby;
    int night;
    int action_points;
    char notice[128];
} GameState;

void game_init(GameState *game);
void game_update(GameState *game, float delta);
void game_draw(const GameState *game);
void game_shutdown(GameState *game);

#endif
