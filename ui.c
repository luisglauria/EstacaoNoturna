#include "ui.h"
#include "raylib.h"

static Color cream(void) { return (Color){224, 208, 166, 255}; }

void ui_draw_hud(int night, int action_points, Equipment nearby) {
    DrawRectangle(0, 0, 1120, 68, (Color){17, 19, 28, 255});
    DrawText("ESTAÇÃO NOTURNA", 28, 18, 24, cream());
    DrawText("NOITE", 430, 15, 13, (Color){143, 148, 160, 255});
    DrawText(TextFormat("%02d", night), 430, 31, 24, (Color){218, 120, 87, 255});
    DrawText("PONTOS DE AÇÃO", 540, 15, 13, (Color){143, 148, 160, 255});
    DrawText(TextFormat("%02d", action_points), 570, 31, 24, (Color){218, 190, 103, 255});
    if (nearby != EQUIPMENT_NONE) {
        DrawRectangle(820, 18, 258, 34, (Color){47, 55, 61, 255});
        DrawText("[E] INTERAGIR", 875, 27, 16, (Color){177, 218, 183, 255});
    }
}

void ui_draw_panel(Panel panel, const RadioList *list, const RadioNode *selected, const char *notice) {
    if (panel == PANEL_NONE) return;
    DrawRectangle(180, 105, 760, 490, (Color){18, 21, 31, 248});
    DrawRectangleLinesEx((Rectangle){180, 105, 760, 490}, 3, (Color){190, 145, 91, 255});
    const char *title = panel == PANEL_TRANSMITTER ? "TRANSMISSOR // PROGRAMAÇÃO" :
                        panel == PANEL_RECEIVER ? "RECEPTOR // MENSAGENS" :
                        panel == PANEL_DESK ? "BANCADA // PISTAS" : "PAINEL // FREQUÊNCIA";
    DrawText(title, 215, 135, 24, cream());
    DrawText("[ESC] fechar", 795, 138, 14, (Color){143, 148, 160, 255});

    if (panel == PANEL_TRANSMITTER) {
        DrawText("ORDEM ATUAL", 220, 180, 15, (Color){143, 148, 160, 255});
        int y = 210, index = 1;
        for (const RadioNode *node = list->head; node; node = node->next, index++) {
            Color background = node == selected ? (Color){77, 70, 52, 255} : (Color){38, 43, 54, 255};
            DrawRectangle(220, y - 5, 650, 48, background);
            DrawText(TextFormat("%d", index), 235, y + 9, 18, (Color){218, 190, 103, 255});
            DrawText(node->data.title, 275, y + 3, 17, cream());
            DrawText(TextFormat("prazo %02d  | custo %d", node->data.deadline, node->data.cost), 275, y + 23, 12, (Color){157, 166, 172, 255});
            if (node->data.urgent) DrawText("URGENTE", 770, y + 12, 12, (Color){225, 112, 93, 255});
            y += 57;
        }
        DrawText("[TAB] selecionar   [P] priorizar urgentes   [R] remover", 220, 540, 14, (Color){177, 218, 183, 255});
    } else {
        const char *body = panel == PANEL_RECEIVER ? "Mensagens recebidas ficam registradas aqui.\nCompare horários, códigos e fontes antes de transmitir." :
                           panel == PANEL_DESK ? "Documentos e pistas poderão ser comparados\npara encontrar contradições entre as fontes." :
                           "Ajuste a frequência para reduzir o rastreamento.\nEsta interação será conectada aos pontos de ação.";
        DrawText(body, 230, 215, 20, (Color){190, 198, 194, 255});
    }
    if (notice && notice[0]) DrawText(notice, 220, 565, 15, (Color){218, 190, 103, 255});
}
