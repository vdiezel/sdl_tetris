#include <SDL2/SDL.h>
#include "./game_state.h"

void render_field_block(SDL_Renderer *renderer, int row, int col) {
  SDL_Rect block = {
    FIELD_ORIGIN_X + col * BLOCK_WIDTH,
    FIELD_ORIGIN_Y + row  * BLOCK_WIDTH,
    BLOCK_WIDTH,
    BLOCK_WIDTH,
  };
  SDL_RenderFillRect(renderer, &block);
}

void render_piece(SDL_Renderer *renderer, GameState *state) {
  for (int i = 0; i < 4; i++) {
    int *tile = state->curr_piece[i];
    int row_idx = tile[0];
    int col_idx = tile[1];

    render_field_block(renderer, row_idx, col_idx);
  }
}

void render_preview(SDL_Renderer *renderer, GameState *state) {
  SDL_Rect preview_frame = {
    PREVIEW_ORIGIN_X,
    PREVIEW_ORIGIN_Y,
    PREVIEW_WIDTH,
    PREVIEW_HEIGHT,
  };

  SDL_RenderDrawRect(renderer, &preview_frame);

  const int (*piece_shape)[4] = get_shape(state->next_piece_type);
  const float *center = get_center(state->next_piece_type);

  for (int m = 0; m < 4; m++) {
    for (int n = 0; n < 4; n++) {
      int piece_block = piece_shape[m][n];

      int x = PREVIEW_ORIGIN_X + PREVIEW_WIDTH / 2 +
        - (int)(center[0] * BLOCK_WIDTH);
      int y = PREVIEW_ORIGIN_Y + PREVIEW_HEIGHT / 2 +
        - (int)(center[1] * BLOCK_WIDTH);

      if (piece_block) {
        SDL_Rect block = {
          x + n * BLOCK_WIDTH,
          y + m * BLOCK_WIDTH,
          BLOCK_WIDTH,
          BLOCK_WIDTH,
        };
        SDL_RenderFillRect(renderer, &block);
      }
    }
  }
}

int get_vertical_space(GameState *state, int row_idx, int col_idx) {
  int curr_idx = row_idx;

  while(!state->field[curr_idx + 1][col_idx] && (curr_idx + 1) < FIELD_ROWS) {
    curr_idx++;
  }

  return curr_idx - row_idx;
}

void render_ghost(SDL_Renderer *renderer, GameState *state) {
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 50);

  int max_distance = 1000;

  for (int i = 0; i < 4; i++) {
    int *tile = state->curr_piece[i];
    int row_idx = tile[0];
    int col_idx = tile[1];

    int curr_dist = get_vertical_space(state, row_idx, col_idx);

    if (curr_dist < max_distance) {
      max_distance = curr_dist;
    }
  }

  for (int i = 0; i < 4; i++) {
    int *tile = state->curr_piece[i];
    int row_idx = tile[0];
    int col_idx = tile[1];
    render_field_block(renderer, row_idx + max_distance, col_idx);
  }
}

void renderField(SDL_Renderer *renderer, GameState *state) {
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

  SDL_Rect field_rect = {
    FIELD_ORIGIN_X - 1,
    FIELD_ORIGIN_Y - 1,
    FIELD_WIDTH + 2,
    FIELD_HEIGHT + 2,
  };

  SDL_RenderDrawRect(renderer, &field_rect);

  for (int i = 0; i < FIELD_ROWS; i++) {
    for (int j = 0; j < FIELD_COLUMNS; j++) {
      int has_block = state->field[i][j];

      if (has_block) {
        render_field_block(renderer, i, j);
      }
    }
  }

  render_preview(renderer, state);

  if (state->has_piece) {
    render_piece(renderer, state);

    if (SHOW_GHOST) {
      render_ghost(renderer, state);
    }
  }
}

void render(SDL_Renderer *renderer, GameState *state) {
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderClear(renderer);

  renderField(renderer, state);

  SDL_RenderPresent(renderer);
}

