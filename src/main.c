// Tetris game using the Original Rotation System
#include <stdio.h>
#include <SDL2/SDL.h>
#include "./constants.h"

int CAP_FRAME_RATE = TRUE;

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

int game_is_running = FALSE;
int has_piece = FALSE;
int last_frame_time = 0;
int field[FIELD_ROWS][FIELD_COLUMNS];

int curr_piece[4][2] = {{-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}};
int last_gravity = 0;

int PIECE_T[4][3][3] = {
  {
    {0, 0, 0},
    {1, 1, 1},
    {0, 1, 0},
  },
  {
    {0, 1, 0},
    {0, 1, 1},
    {0, 1, 0},
  },
  {
    {0, 1, 0},
    {1, 1, 1},
    {0, 0, 0},
  },
  {
    {0, 1, 0},
    {1, 1, 0},
    {0, 1, 0},
  },
};

int initialize_window(void) {
  if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
    fprintf(stderr, "Error initializinh SDL.\n");
    return FALSE;
  }

  window = SDL_CreateWindow(
    NULL,
    -1,
    -1,
    WINDOW_WIDTH,
    WINDOW_HEIGHT,
    SDL_WINDOW_BORDERLESS
  );

  if (!window) {
    fprintf(stderr, "Error creating SDL Window.\n");
    return FALSE;
  }

  renderer = SDL_CreateRenderer(window, -1, 0);

  if (!renderer) {
    fprintf(stderr, "Error creating SDL Renderer.\n");
    return FALSE;
  }

  return TRUE;
}

void setup() {
  for (int i = 0; i < FIELD_ROWS; i++) {
    for (int j = 0; j < FIELD_COLUMNS; j++) {
      field[i][j] = 0;
    }
  }
}

int valid_move(int row_dir, int col_dir) {
  for (int i = 0; i < 4; i++) {
    int row_idx = curr_piece[i][0];
    if (row_dir == 1 && row_idx >= (FIELD_ROWS - 1)) return FALSE;

    int col_idx = curr_piece[i][1];
    if (col_dir == 1 && col_idx >= (FIELD_COLUMNS - 1)) return FALSE;
    if (col_dir == -1 && col_idx <= 0) return FALSE;
  }

  return TRUE;
}

void moveLateral(int dir) {
  if (has_piece && valid_move(0, dir)) {
    for (int i = 0; i < 4; i++) {
      curr_piece[i][1] = curr_piece[i][1] + dir;
    }
  }
}

void soft_drop() {
  if (has_piece && valid_move(-1, 0)) {
    for (int i = 0; i < 4; i++) {
      curr_piece[i][0] = curr_piece[i][0] + 1;
    }
  }
}

void handle_key_down(SDL_Event event) {
  switch (event.key.keysym.sym) {
    case SDLK_ESCAPE:
      game_is_running = FALSE;
      break;
    case SDLK_a:
      moveLateral(-1);
      break;
    case SDLK_d:
      moveLateral(1);
      break;
    case SDLK_s:
      soft_drop();
      break;
  }
}

void process_inputs() {
  SDL_Event event;
  SDL_PollEvent(&event);

  switch (event.type) {
    case SDL_QUIT:
      game_is_running = FALSE;
      break;
    case SDL_KEYDOWN:
      handle_key_down(event);
      break;
  }
}

void spawn_piece() {
  int tile_counter = 0;

  for (int m = 0; m < 3; m++) {
    for (int n = 0; n < 3; n++) {
      int field_block = field[m][n + 4];
      int piece_block = PIECE_T[0][m][n];

      if (piece_block && field_block) {
        // game over
        return;
      }

      if (piece_block) {
        curr_piece[tile_counter][0] = m;
        curr_piece[tile_counter][1] = n + 4;
        tile_counter++;
      }
    }
  }

  has_piece = TRUE;
}

void merge_piece() {
  for (int i = 0; i < 4; i++) {
    int row_idx = curr_piece[i][0];
    int col_idx = curr_piece[i][1];
    field[row_idx][col_idx] = 1;
  }
}

void reset_current_piece() {
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 2; j++) {
      curr_piece[i][j] = -1;
    }
  }
  has_piece = FALSE;
}

void apply_gravity() {
  if (!has_piece) return;

  if (valid_move(1, 0)) {
    for (int i = 0; i < 4; i++) {
      curr_piece[i][0] = curr_piece[i][0] + 1;
    }
  } else {
    merge_piece();
    reset_current_piece();
  }

  last_gravity = SDL_GetTicks();
}

void cap_frame_rate() {
  int time_to_wait = FRAME_TARGET_TIME - (SDL_GetTicks() - last_frame_time);

  if (time_to_wait > 0) {
    SDL_Delay(time_to_wait);
  }
}

void update() {

  if (CAP_FRAME_RATE) {
    cap_frame_rate();
  }

  if (!has_piece) {
    spawn_piece();
  } else if ((SDL_GetTicks() - last_gravity) > 500) {
    apply_gravity();
  }

  //float delta_time = (SDL_GetTicks() - last_frame_time) / 1000.0f;
  last_frame_time = SDL_GetTicks();
}

void renderFieldBlock(int row, int col) {
  SDL_Rect block = {
    FIELD_ORIGIN_X + col * BLOCK_WIDTH,
    FIELD_ORIGIN_Y + row  * BLOCK_WIDTH,
    BLOCK_WIDTH,
    BLOCK_WIDTH,
  };
  SDL_RenderFillRect(renderer, &block);
}

void renderField() {
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
      int has_block = field[i][j];

      if (has_block) {
        renderFieldBlock(i, j);
      }
    }
  }

  if (has_piece) {
    for (int i = 0; i < 4; i++) {
      int *tile = curr_piece[i];
      int row_idx = tile[0];
      int col_idx = tile[1];

      renderFieldBlock(row_idx, col_idx);
    }
  }
}

void render() {
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderClear(renderer);

  renderField();

  SDL_RenderPresent(renderer);
}

void destroy_window() {
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
}

int main() {
  game_is_running = initialize_window();

  setup();

  while (game_is_running) {
    process_inputs();
    update();
    render();
  }

  destroy_window();

  return 0;
}
