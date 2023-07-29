// Tetris game using the Original Rotation System
#include <stdio.h>
#include <SDL2/SDL.h>
#include "./constants.h"
#include "./pieces.h"
#include "./game_state.h"
#include "./render.h"

typedef enum {
  LEFT,
  RIGHT,
  SOFT_DROP,
  ROTATE,
} Movement;

void reset_current_piece(GameState *state) {
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 2; j++) {
      state->curr_piece[i][j] = -1;
    }
  }
  state->curr_rot = 0;
  state->has_piece = FALSE;
}

int is_valid_position(int new_piece_position[4][2], GameState *state) {
  for (int i = 0; i < 4; i++) {
    int row_idx = new_piece_position[i][0];
    if (row_idx >= FIELD_ROWS) return FALSE;

    int col_idx = new_piece_position[i][1];
    if (col_idx >= FIELD_COLUMNS) return FALSE;
    if (col_idx < 0) return FALSE;

    if (state->field[row_idx][col_idx]) return FALSE;
  }

  return TRUE;
}

void copy_piece(int dest[4][2], GameState *state) {
  for (int i = 0; i < 4; i++) {
    dest[i][0] = state->curr_piece[i][0];
    dest[i][1] = state->curr_piece[i][1];
  }
}

void apply_new_position(int new_piece_position[4][2], GameState *state) {
  for (int i = 0; i < 4; i++) {
    state->curr_piece[i][0] = new_piece_position[i][0];
    state->curr_piece[i][1] = new_piece_position[i][1];
  }
}

void remove_row(int row_idx, GameState *state) {
  for (int i = (row_idx - 1); i >= 0; i--) {
    for (int j = 0; j < FIELD_COLUMNS; j++) {
      state->field[i + 1][j] = state->field[i][j];
    }
  }

  for (int j = 0; j < FIELD_COLUMNS; j++) {
    state->field[0][j] = 0;
  }
}

void remove_complete_rows(GameState *state) {
  for (int i = 0; i < FIELD_ROWS; i++) {
    int is_complete = TRUE;

    for (int j = 0; j < FIELD_COLUMNS; j++) {
      if (!state->field[i][j]) is_complete = FALSE;
    }

    if (is_complete) {
      remove_row(i, state);
    }
  }
}

void merge_piece(GameState *state) {
  for (int i = 0; i < 4; i++) {
    int row_idx = state->curr_piece[i][0];
    int col_idx = state->curr_piece[i][1];
    state->field[row_idx][col_idx] = 1;
  }

  remove_complete_rows(state);
}

void move(Movement move, GameState *state) {
  if (!state->has_piece) return;

  int piece_copy[4][2];
  copy_piece(piece_copy, state);

  for (int i = 0; i < 4; i++) {
    switch (move) {
      case LEFT:
        piece_copy[i][1] = state->curr_piece[i][1] - 1;
        break;
      case RIGHT:
        piece_copy[i][1] = state->curr_piece[i][1] + 1;
        break;
      case SOFT_DROP:
        piece_copy[i][0] = state->curr_piece[i][0] + 1;
        break;
      case ROTATE: {
        const int (*rotation_offsets)[4][2] = get_rotation_offsets(state->piece_type);
        piece_copy[i][0] = state->curr_piece[i][0] + rotation_offsets[state->curr_rot][i][0];
        piece_copy[i][1] = state->curr_piece[i][1] + rotation_offsets[state->curr_rot][i][1];
        break;
      }
    }
  }

  if (is_valid_position(piece_copy, state)) {
    apply_new_position(piece_copy, state);
    if (move == ROTATE) {
      state->curr_rot = (state->curr_rot + 1) % 4;
    }
  } else if (move == SOFT_DROP) {
    merge_piece(state);
    reset_current_piece(state);
  }
}

void handle_key_down(SDL_Event event, GameState *state) {
  switch (event.key.keysym.sym) {
    case SDLK_ESCAPE:
      state->game_is_running = FALSE;
      break;
    case SDLK_a:
      move(LEFT, state);
      break;
    case SDLK_d:
      move(RIGHT, state);
      break;
    case SDLK_w:
      move(ROTATE, state);
      break;
    case SDLK_s:
      move(SOFT_DROP, state);
      break;
  }
}

void process_inputs(GameState *state) {
  SDL_Event event;
  SDL_PollEvent(&event);

  switch (event.type) {
    case SDL_QUIT:
      state->game_is_running = FALSE;
      break;
    case SDL_KEYDOWN:
      handle_key_down(event, state);
      break;
  }
}

void spawn_piece(GameState *state) {
  int tile_counter = 0;

  state->piece_type = state->next_piece_type;

  srand((unsigned int)(SDL_GetTicks() / 1000));
  state->next_piece_type = rand() % NUM_PIECE_TYPES;

  const int (*piece_shape)[4] = get_shape(state->piece_type);

  for (int m = 0; m < 4; m++) {
    for (int n = 0; n < 4; n++) {
      int field_block = state->field[m][n + 4];
      int piece_block = piece_shape[m][n];

      if (piece_block) {

        if (field_block) {
          state->game_is_running = FALSE;
          return;
        }

        state->curr_piece[tile_counter][0] = m;
        state->curr_piece[tile_counter][1] = n + 4;
        tile_counter++;
      }
    }
  }

  state->has_piece = TRUE;
}

void apply_gravity(GameState *state) {
  if (!state->has_piece) return;
  move(SOFT_DROP, state);

  state->last_gravity = SDL_GetTicks();
}

void cap_frame_rate(GameState *state) {
  int time_to_wait = FRAME_TARGET_TIME - (SDL_GetTicks() - state->last_frame_time);

  if (time_to_wait > 0) {
    SDL_Delay(time_to_wait);
  }
}

void update(GameState *state) {

  if (CAP_FRAME_RATE) {
    cap_frame_rate(state);
  }

  if (!state->has_piece) {
    spawn_piece(state);
  } else if ((SDL_GetTicks() - state->last_gravity) > GRAVITY_TICK_INTERVAL_MS) {
    apply_gravity(state);
  }

  state->last_frame_time = SDL_GetTicks();
}

void destroy(SDL_Window *window, SDL_Renderer *renderer) {
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
}

void setup_state(GameState *state) {
  for (int i = 0; i < FIELD_ROWS; i++) {
    for (int j = 0; j < FIELD_COLUMNS; j++) {
      state->field[i][j] = 0;
    }
  }

  srand((unsigned int)(SDL_GetTicks() / 1000));
  state->piece_type = rand() % NUM_PIECE_TYPES;

  srand((unsigned int)(SDL_GetTicks() / 1000));
  state->next_piece_type = rand() % NUM_PIECE_TYPES;

  reset_current_piece(state);
}

int setup_window(SDL_Window **window) {
  *window = SDL_CreateWindow(
    NULL,
    -1,
    -1,
    WINDOW_WIDTH,
    WINDOW_HEIGHT,
    SDL_WINDOW_BORDERLESS
  );

  if (!window) {
    fprintf(stderr, "Error creating SDL Window.\n");
    return 1;
  }

  return 0;
}

int setup_renderer(SDL_Window *window, SDL_Renderer **renderer) {
  *renderer = SDL_CreateRenderer(window, -1, 0);

  if (!renderer) {
    fprintf(stderr, "Error creating SDL Renderer.\n");
    return 1;
  }

  SDL_SetRenderDrawBlendMode(*renderer, SDL_BLENDMODE_BLEND);

  return 0;
}

int main() {
  SDL_Window* window = NULL;
  SDL_Renderer* renderer = NULL;

  if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
    fprintf(stderr, "Error initializinh SDL.\n");
    return 1;
  }

  if (setup_window(&window) != 0) {
    return 2;
  };

  if (setup_renderer(window, &renderer) != 0) {
    return 3;
  };

  GameState state = {
    .game_is_running = TRUE,
    .last_frame_time = 0,
    .last_gravity = 0,
  };

  setup_state(&state);

  while (state.game_is_running) {
    process_inputs(&state);
    update(&state);
    render(renderer, &state);
  }

  destroy(window, renderer);

  return 0;
}
