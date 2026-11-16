#ifndef CABIN_H
#define CABIN_H

#include "raylib.h"

typedef enum { EQUIPMENT_NONE, EQUIPMENT_RECEIVER, EQUIPMENT_DESK, EQUIPMENT_TRANSMITTER, EQUIPMENT_FREQUENCY } Equipment;

Rectangle cabin_bounds(void);
Equipment cabin_near_equipment(Vector2 player_position);
void cabin_draw(void);

#endif
