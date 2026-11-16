#include "game.h"
#include "raylib.h"
#include <stdio.h>

static void set_notice(GameState *game, const char *message) {
    snprintf(game->notice, sizeof(game->notice), "%s", message);
}

void game_init(GameState *game) {
    radio_list_init(&game->transmissions);
    game->selected = NULL;
    game->panel = PANEL_NONE;
    game->night = 1;
    game->action_points = 8;
    game->notice[0] = '\0';
    player_init(&game->player, (Vector2){550, 330});
    radio_list_push_back(&game->transmissions, (Transmission){"Palavra-código: ORION", "ORION", 3, 1, true});
    radio_list_push_back(&game->transmissions, (Transmission){"Rota de fuga // setor leste", "R-17", 7, 2, false});
    radio_list_push_back(&game->transmissions, (Transmission){"Denúncia: arquivos do Ministério", "ARQ", 5, 2, true});
    game->selected = game->transmissions.head;
}

static void cycle_selection(GameState *game) {
    if (!game->selected) return;
    game->selected = game->selected->next ? game->selected->next : game->transmissions.head;
}

static void interact(GameState *game) {
    switch (game->nearby) {
        case EQUIPMENT_RECEIVER: game->panel = PANEL_RECEIVER; set_notice(game, "Nenhuma mensagem nova. O silêncio também é uma pista."); break;
        case EQUIPMENT_DESK: game->panel = PANEL_DESK; set_notice(game, "Documentos aguardando comparação."); break;
        case EQUIPMENT_TRANSMITTER: game->panel = PANEL_TRANSMITTER; set_notice(game, "Organize a sequência antes de transmitir."); break;
        case EQUIPMENT_FREQUENCY: game->panel = PANEL_FREQUENCY; set_notice(game, "Frequência estável. Ainda não consome ação."); break;
        default: break;
    }
}

void game_update(GameState *game, float delta) {
    if (game->panel == PANEL_NONE) {
        player_update(&game->player, delta, cabin_bounds());
        game->nearby = cabin_near_equipment(game->player.position);
        if (IsKeyPressed(KEY_E)) interact(game);
    } else {
        if (IsKeyPressed(KEY_ESCAPE)) game->panel = PANEL_NONE;
        if (game->panel == PANEL_TRANSMITTER) {
            if (IsKeyPressed(KEY_TAB)) cycle_selection(game);
            if (IsKeyPressed(KEY_P)) { radio_list_insertion_sort(&game->transmissions); game->selected = game->transmissions.head; set_notice(game, "Insertion Sort aplicado: menor prazo primeiro."); }
            if (IsKeyPressed(KEY_R) && game->selected) { RadioNode *next = game->selected->next ? game->selected->next : game->selected->prev; radio_list_remove(&game->transmissions, game->selected); game->selected = next; set_notice(game, "Transmissão removida da programação."); }
        }
    }
}

void game_draw(const GameState *game) {
    ClearBackground((Color){12, 14, 20, 255});
    cabin_draw();
    player_draw(&game->player);
    ui_draw_hud(game->night, game->action_points, game->nearby);
    ui_draw_panel(game->panel, &game->transmissions, game->selected, game->notice);
}

void game_shutdown(GameState *game) { radio_list_clear(&game->transmissions); }
