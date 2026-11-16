#include "cabin.h"

static const Rectangle receiver = {130, 155, 170, 100};
static const Rectangle desk = {405, 440, 310, 90};
static const Rectangle transmitter = {770, 160, 170, 105};
static const Rectangle frequency = {825, 400, 120, 150};

Rectangle cabin_bounds(void) { return (Rectangle){75, 85, 970, 535}; }

Equipment cabin_near_equipment(Vector2 p) {
    if (CheckCollisionPointRec(p, receiver)) return EQUIPMENT_RECEIVER;
    if (CheckCollisionPointRec(p, desk)) return EQUIPMENT_DESK;
    if (CheckCollisionPointRec(p, transmitter)) return EQUIPMENT_TRANSMITTER;
    if (CheckCollisionPointRec(p, frequency)) return EQUIPMENT_FREQUENCY;
    return EQUIPMENT_NONE;
}

void cabin_draw(void) {
    Rectangle room = cabin_bounds();
    DrawRectangleRec(room, (Color){28, 31, 43, 255});
    DrawRectangleLinesEx(room, 5, (Color){125, 94, 67, 255});
    DrawRectangle(95, 105, 930, 18, (Color){61, 47, 53, 255});
    DrawRectangle(95, 585, 930, 18, (Color){61, 47, 53, 255});
    DrawRectangle(98, 145, 18, 410, (Color){61, 47, 53, 255});
    DrawRectangle(1005, 145, 18, 410, (Color){61, 47, 53, 255});

    DrawRectangleRec(receiver, (Color){48, 61, 75, 255});
    DrawRectangle(145, 170, 140, 55, (Color){10, 16, 25, 255});
    DrawText("RECEPTOR", 155, 232, 18, (Color){184, 222, 194, 255});
    DrawCircle(255, 198, 12, (Color){194, 155, 71, 255});

    DrawRectangleRec(transmitter, (Color){73, 52, 58, 255});
    DrawRectangle(790, 178, 130, 48, (Color){24, 20, 28, 255});
    DrawText("TRANSMISSOR", 780, 238, 17, (Color){230, 176, 113, 255});
    DrawCircle(810, 200, 7, (Color){188, 65, 68, 255});

    DrawRectangleRec(desk, (Color){92, 65, 52, 255});
    DrawRectangle(430, 410, 260, 45, (Color){160, 119, 78, 255});
    DrawText("BANCADA DE INVESTIGAÇÃO", 425, 468, 18, (Color){225, 196, 146, 255});
    DrawRectangleRec(frequency, (Color){45, 69, 66, 255});
    DrawText("FREQUÊNCIA", 812, 565, 16, (Color){153, 222, 190, 255});

    DrawRectangle(465, 110, 180, 22, (Color){113, 79, 66, 255});
    DrawText("JANELA", 515, 137, 16, (Color){162, 151, 142, 255});
    DrawRectangle(900, 285, 80, 30, (Color){102, 71, 54, 255});
    DrawText("PORTA", 914, 291, 14, (Color){224, 183, 132, 255});
}
