#include "game.h"
#include "raylib.h"

int main(void) {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(1120, 680, "Estacao Noturna: Sinal de Ruptura");
    SetTargetFPS(60);

    GameState game;
    game_init(&game);
    while (!WindowShouldClose()) {
        game_update(&game, GetFrameTime());
        BeginDrawing();
        game_draw(&game);
        EndDrawing();
    }
    game_shutdown(&game);
    CloseWindow();
    return 0;
}
