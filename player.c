#include "player.h"
#include "pixel.h"
#include "raymath.h"
#include "sprites.h"
#include <math.h>

#define PLAYER_SPEED 170.0f
#define PASSOS_POR_SEGUNDO 7.0f

void player_init(Player *player, Vector2 position) {
    player->position = position;
    player->facing = (Vector2){0, 1};
    player->speed = PLAYER_SPEED;
    player->walk_time = 0.0f;
    player->moving = false;
}

/* A hitbox fica nos pés: assim a cabeça pode "passar na frente" dos móveis de cima. */
Rectangle player_hitbox_at(Vector2 position) { return (Rectangle){position.x - 11, position.y - 2, 22, 16}; }

static Vector2 read_direction(void) {
    Vector2 direction = {0, 0};
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) direction.y -= 1.0f;
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) direction.y += 1.0f;
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) direction.x -= 1.0f;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) direction.x += 1.0f;
    return Vector2Length(direction) > 0 ? Vector2Normalize(direction) : direction;
}

/* Move um eixo por vez: se um eixo bate, o outro continua e o jogador desliza na parede. */
static void try_move(Player *player, Vector2 step, BlockedCheck is_blocked) {
    Vector2 target = Vector2Add(player->position, step);
    if (!is_blocked(player_hitbox_at(target))) player->position = target;
}

void player_update(Player *player, float delta, BlockedCheck is_blocked) {
    Vector2 direction = read_direction();
    player->moving = Vector2Length(direction) > 0;
    if (!player->moving) return;
    player->facing = direction;
    player->walk_time += delta;
    float distance = player->speed * delta;
    try_move(player, (Vector2){direction.x * distance, 0}, is_blocked);
    try_move(player, (Vector2){0, direction.y * distance}, is_blocked);
}

int player_feet_pixel(const Player *player) { return px(player->position.y + 14); }

/* Nas diagonais vale o eixo horizontal: o perfil mostra melhor para onde ele anda. */
static Direcao direction_from_facing(Vector2 facing) {
    if (fabsf(facing.x) > 0.3f) return facing.x < 0 ? DIRECAO_ESQUERDA : DIRECAO_DIREITA;
    return facing.y < 0 ? DIRECAO_CIMA : DIRECAO_BAIXO;
}

void player_draw(const Player *player) {
    int x = px(player->position.x), pes = player_feet_pixel(player);
    DrawEllipse(x, pes, 7, 2, (Color){0, 0, 0, 120});
    int passo = player->moving ? (int)(player->walk_time * PASSOS_POR_SEGUNDO) % 4 : 0;
    sprite_operador(x, pes, direction_from_facing(player->facing), passo);
}

/* Pequeno passo audível: devolve true no instante em que um pé toca o chão. */
bool player_step_landed(const Player *player, float delta) {
    if (!player->moving) return false;
    int antes = (int)((player->walk_time - delta) * PASSOS_POR_SEGUNDO);
    int agora = (int)(player->walk_time * PASSOS_POR_SEGUNDO);
    return antes != agora && agora % 2 == 1;
}
