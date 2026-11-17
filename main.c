#include "app.h"
#include "audio.h"
#include "cabin.h"
#include "pixel.h"
#include "raylib.h"
#include "render.h"
#include "sprites.h"

int main(void) {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);
    InitWindow(VIRTUAL_WIDTH, VIRTUAL_HEIGHT, "Estação Noturna: Sinal de Ruptura");
    SetWindowMinSize(640, 360);
    /* ESC fecha painéis e abre a pausa; sem isto a raylib usaria ESC para fechar a janela inteira. */
    SetExitKey(KEY_NULL);
    SetTargetFPS(60);
    render_init();
    pixel_iniciar();
    sprites_iniciar();
    cabin_iniciar(); /* depois dos sprites: a fotografia da parede entra na camada fixa */
    audio_iniciar();

    App app;
    app_iniciar(&app);
    while (!WindowShouldClose() && !app_deve_sair(&app)) {
        render_begin_frame();
        app_quadro(&app);
        render_end_frame();
    }
    app_encerrar(&app);
    audio_encerrar();
    cabin_encerrar();
    sprites_encerrar();
    pixel_encerrar();
    render_shutdown();
    CloseWindow();
    return 0;
}
