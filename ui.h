#ifndef UI_H
#define UI_H

#include "cabin.h"
#include "radio_list.h"

typedef enum { PANEL_NONE, PANEL_RECEIVER, PANEL_DESK, PANEL_TRANSMITTER, PANEL_FREQUENCY } Panel;

void ui_draw_hud(int night, int action_points, Equipment nearby);
void ui_draw_panel(Panel panel, const RadioList *list, const RadioNode *selected, const char *notice);

#endif
