// Tetris game using the Original Rotation System
#include <stdio.h>
#include <SDL2/SDL.h>
#include "./constants.h"
#include "./pieces.h"

int CAP_FRAME_RATE = TRUE;
int SHOW_GHOST = TRUE;

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

int game_is_running = FALSE;
int has_piece = FALSE;
int last_frame_time = 0;
int field[FIELD_ROWS][FIELD_COLUMNS];

// this holds the field indexes if the tiles of the current piece
int curr_piece[4][2] = {{-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}};
int last_gravity = 0;
int curr_rot = 0;
PieceType piece_type = T;
PieceType next_piece_type = T;

typedef enum {
  LEFT,
  RIGHT,
  SOFT_DROP,
  ROTATE,
} Movement;

int initialize_window() {
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

  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

  return TRUE;
}

void setup() {
  for (int i = 0; i < FIELD_ROWS; i++) {
    for (int j = 0; j < FIELD_COLUMNS; j++) {
      field[i][j] = 0;
    }
  }

  srand((unsigned int)(SDL_GetTicks() / 1000));
  piece_type = rand() % NUM_PIECE_TYPES;

  srand((unsigned int)(SDL_GetTicks() / 1000));
  next_piece_type = rand() % NUM_PIECE_TYPES;
}

int is_valid_position(int new_piece_position[4][2]) {
  for (int i = 0; i < 4; i++) {
    int row_idx = new_piece_position[i][0];
    if (row_idx >= FIELD_ROWS) return FALSE;

    int col_idx = new_piece_position[i][1];
    if (col_idx >= FIELD_COLUMNS) return FALSE;
    if (col_idx < 0) return FALSE;

    if (field[row_idx][col_idx]) return FALSE;
  }

  return TRUE;
}

void copy_piece(int dest[4][2]) {
  for (int i = 0; i < 4; i++) {
    dest[i][0] = curr_piece[i][0];
    dest[i][1] = curr_piece[i][1];
  }
}

void apply_new_position(int new_piece_position[4][2]) {
  for (int i = 0; i < 4; i++) {
    curr_piece[i][0] = new_piece_position[i][0];
    curr_piece[i][1] = new_piece_position[i][1];
  }
}

void remove_row(int row_idx) {
  for (int i = (row_idx - 1); i >= 0; i--) {
    for (int j = 0; j < FIELD_COLUMNS; j++) {
      field[i + 1][j] = field[i][j];
    }
  }

  for (int j = 0; j < FIELD_COLUMNS; j++) {
    field[0][j] = 0;
  }
}

void remove_complete_rows() {
  for (int i = 0; i < FIELD_ROWS; i++) {
    int is_complete = TRUE;

    for (int j = 0; j < FIELD_COLUMNS; j++) {
      if (!field[i][j]) is_complete = FALSE;
    }

    if (is_complete) {
      remove_row(i);
    }
  }
}

void merge_piece() {
  for (int i = 0; i < 4; i++) {
    int row_idx = curr_piece[i][0];
    int col_idx = curr_piece[i][1];
    field[row_idx][col_idx] = 1;
  }

  remove_complete_rows();
}

void reset_current_piece() {
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 2; j++) {
      curr_piece[i][j] = -1;
    }
  }
  curr_rot = 0;
  has_piece = FALSE;
}

void move(Movement move) {
  if (!has_piece) return;

  int piece_copy[4][2];
  copy_piece(piece_copy);

  for (int i = 0; i < 4; i++) {
    switch (move) {
      case LEFT:
        piece_copy[i][1] = curr_piece[i][1] - 1;
        break;
      case RIGHT:
        piece_copy[i][1] = curr_piece[i][1] + 1;
        break;
      case SOFT_DROP:
        piece_copy[i][0] = curr_piece[i][0] + 1;
        break;
      case ROTATE: {
        const int (*rotation_offsets)[4][2] = get_rotation_offsets(piece_type);
        piece_copy[i][0] = curr_piece[i][0] + rotation_offsets[curr_rot][i][0];
        piece_copy[i][1] = curr_piece[i][1] + rotation_offsets[curr_rot][i][1];
        break;
      }
    }
  }

  if (is_valid_position(piece_copy)) {
    apply_new_position(piece_copy);
    if (move == ROTATE) {
      curr_rot = (curr_rot + 1) % 4;
    }
  } else if (move == SOFT_DROP) {
    merge_piece();
    reset_current_piece();
  }
}

void handle_key_down(SDL_Event event) {
  switch (event.key.keysym.sym) {
    case SDLK_ESCAPE:
      game_is_running = FALSE;
      break;
    case SDLK_a:
      move(LEFT);
      break;
    case SDLK_d:
      move(RIGHT);
      break;
    case SDLK_w:
      move(ROTATE);
      break;
    case SDLK_s:
      move(SOFT_DROP);
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

  piece_type = next_piece_type;

  srand((unsigned int)(SDL_GetTicks() / 1000));
  next_piece_type = rand() % NUM_PIECE_TYPES;

  const int (*piece_shape)[4] = get_shape(piece_type);

  for (int m = 0; m < 4; m++) {
    for (int n = 0; n < 4; n++) {
      int field_block = field[m][n + 4];
      int piece_block = piece_shape[m][n];

      if (piece_block) {

        if (field_block) {
          game_is_running = FALSE;
          return;
        }

        curr_piece[tile_counter][0] = m;
        curr_piece[tile_counter][1] = n + 4;
        tile_counter++;
      }
    }
  }

  has_piece = TRUE;
}

void apply_gravity() {
  if (!has_piece) return;
  move(SOFT_DROP);

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

  last_frame_time = SDL_GetTicks();
}

void render_field_block(int row, int col) {
  SDL_Rect block = {
    FIELD_ORIGIN_X + col * BLOCK_WIDTH,
    FIELD_ORIGIN_Y + row  * BLOCK_WIDTH,
    BLOCK_WIDTH,
    BLOCK_WIDTH,
  };
  SDL_RenderFillRect(renderer, &block);
}

void render_piece() {
  for (int i = 0; i < 4; i++) {
    int *tile = curr_piece[i];
    int row_idx = tile[0];
    int col_idx = tile[1];

    render_field_block(row_idx, col_idx);
  }
}

int get_vertical_space(int row_idx, int col_idx) {
  int curr_idx = row_idx;

  while(!field[curr_idx + 1][col_idx] && (curr_idx + 1) < FIELD_ROWS) {
    curr_idx++;
  }

  return curr_idx - row_idx;
}

void render_ghost() {
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 50);

  int max_distance = 1000;

  for (int i = 0; i < 4; i++) {
    int *tile = curr_piece[i];
    int row_idx = tile[0];
    int col_idx = tile[1];

    int curr_dist = get_vertical_space(row_idx, col_idx);

    if (curr_dist < max_distance) {
      max_distance = curr_dist;
    }
  }

  for (int i = 0; i < 4; i++) {
    int *tile = curr_piece[i];
    int row_idx = tile[0];
    int col_idx = tile[1];
    render_field_block(row_idx + max_distance, col_idx);
  }
}

void render_preview() {
  SDL_Rect preview_frame = {
    PREVIEW_ORIGIN_X,
    PREVIEW_ORIGIN_Y,
    PREVIEW_WIDTH,
    PREVIEW_HEIGHT,
  };

  SDL_RenderDrawRect(renderer, &preview_frame);

  const int (*piece_shape)[4] = get_shape(next_piece_type);
  const float *center = get_center(next_piece_type);

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
        render_field_block(i, j);
      }
    }
  }

  render_preview();

  if (has_piece) {
    render_piece();

    if (SHOW_GHOST) {
      render_ghost();
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
