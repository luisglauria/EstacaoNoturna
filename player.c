#include "player.h"

void player_init(Player *player, Vector2 position) { player->position = position; player->speed = 170.0f; }

void player_update(Player *player, float delta, Rectangle bounds) {
    Vector2 direction = {0};
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) direction.y -= 1.0f;
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) direction.y += 1.0f;
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) direction.x -= 1.0f;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) direction.x += 1.0f;
    if (Vector2Length(direction) > 0) direction = Vector2Normalize(direction);
    player->position = Vector2Add(player->position, Vector2Scale(direction, player->speed * delta));
    player->position.x = Clamp(player->position.x, bounds.x + 18, bounds.x + bounds.width - 18);
    player->position.y = Clamp(player->position.y, bounds.y + 18, bounds.y + bounds.height - 18);
}

Rectangle player_hitbox(const Player *player) { return (Rectangle){player->position.x - 12, player->position.y - 12, 24, 24}; }

void player_draw(const Player *player) {
    DrawRectangleV((Vector2){player->position.x - 10, player->position.y - 8}, (Vector2){20, 16}, (Color){205, 185, 137, 255});
    DrawCircle((int)player->position.x, (int)player->position.y - 13, 7, (Color){232, 201, 150, 255});
    DrawRectangle((int)player->position.x - 8, (int)player->position.y - 20, 16, 4, (Color){34, 37, 49, 255});
}
