#ifndef _RENDERER
#define _RENDERER

#include "./game_state.h"
#include <SDL2/SDL.h>

#endif

void render_field_block(SDL_Renderer *renderer, int row, int col);
void render_piece(SDL_Renderer *renderer, GameState *state);
void render_preview(SDL_Renderer *renderer, GameState *state);
int get_vertical_space(GameState *state, int row_idx, int col_idx);
void render_ghost(SDL_Renderer *renderer, GameState *state);
void renderField(SDL_Renderer *renderer, GameState *state);
void render(SDL_Renderer *renderer, GameState *state);
