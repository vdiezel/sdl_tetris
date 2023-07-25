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

// this holds the field indexes if the tiles of the current piece
int curr_piece[4][2] = {{-1, -1}, {-1, -1}, {-1, -1}, {-1, -1}};
int last_gravity = 0;
int curr_rot = 0;

typedef enum {
  T,
  L,
  J,
  S,
  Z,
  O,
  I,
} PieceType;

PieceType piece_type = T;

typedef enum {
  LEFT,
  RIGHT,
  SOFT_DROP,
  ROTATE,
} Movement;

struct Piece {
  int shape[4][4];
  int shape_size;
  int rotation_offsets[4][4][2];
  PieceType type;
};

const static struct Piece piece_t = {
  .shape = {
    {0, 0, 0, 0},
    {1, 1, 1, 0},
    {0, 1, 0, 0},
    {0, 0, 0, 0},
  },
  .rotation_offsets = {
    { {1, 1}, {0, 0}, {-1, -1}, {-1, 1} },
    { {-1, 1}, {0, 0}, {1, -1}, {-1, -1} },
    { {-1, -1}, {0, 0}, {1, 1}, {1, -1} },
    { {1, -1}, {0, 0}, {-1, 1}, {1, 1} },
  },
  .type = T,
};

const static struct Piece piece_l = {
  .shape = {
    {0, 0, 0, 0},
    {1, 1, 1, 0},
    {1, 0, 0, 0},
    {0, 0, 0, 0},
  },
  .rotation_offsets = {
    { {1, 1}, {0, 0}, {-1, -1}, {0, 2} },
    { {-1, 1}, {0, 0}, {1, -1}, {-2, 0} },
    { {-1, -1}, {0, 0}, {1, 1}, {0, -2} },
    { {1, -1}, {0, 0}, {-1, 1}, {2, 0} },
  },
  .type = L,
};

// my poor man's polymorphism
const struct Piece* get_piece(PieceType type) {

  switch (type) {
    case T:
      return &piece_t;
    case L:
      return &piece_l;
  }

}

const int (*get_shape(PieceType type))[4] {
  const struct Piece* piece = get_piece(type);
  return piece->shape;
}

const int (*get_rotation_offsets(PieceType type))[4][2] {
  const struct Piece* piece = get_piece(type);
  return piece->rotation_offsets;
}

struct PIECE_J {
  int shapes[4][3][3];
};

struct PIECE_J piece_j = {
  .shapes = {
    {
      {0, 0, 0},
      {1, 1, 1},
      {0, 0, 1},
    },
    {
      {0, 1, 1},
      {0, 1, 0},
      {0, 1, 0},
    },
    {
      {1, 0, 0},
      {1, 1, 1},
      {0, 0, 0},
    },
    {
      {0, 1, 0},
      {0, 1, 0},
      {1, 1, 0},
    },
  }
};

struct PIECE_S {
  int shapes[2][3][3];
};

struct PIECE_S piece_s = {
  .shapes = {
    {
      {0, 0, 0},
      {0, 1, 1},
      {1, 1, 0},
    },
    {
      {0, 1, 0},
      {0, 1, 1},
      {0, 0, 1},
    },
  }
};

struct PIECE_Z {
  int shapes[2][3][3];
};

struct PIECE_Z piece_z = {
  .shapes = {
    {
      {0, 0, 0},
      {1, 1, 0},
      {0, 1, 1},
    },
    {
      {0, 0, 1},
      {0, 1, 1},
      {0, 1, 0},
    },
  }
};

struct PIECE_O {
  int shapes[1][2][2];
};

struct PIECE_O piece_o = {
  .shapes = {
    {
      {1, 1},
      {1, 1},
    },
  },
};

struct PIECE_I {
  int shapes[2][4][4];
};

struct PIECE_I piece_i = {
  .shapes = {
    {
      {0, 0, 0, 0},
      {1, 1, 1, 1},
      {0, 0, 0, 0},
      {0, 0, 0, 0},
    },
    {
      {0, 0, 1, 0},
      {0, 0, 1, 0},
      {0, 0, 1, 0},
      {0, 0, 1, 0},
    },
  }
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
        const int (*rotation_offsets)[4][2] = get_rotation_offsets(L);
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

  const int (*piece_shape)[4] = get_shape(L);

  for (int m = 0; m < 3; m++) {
    for (int n = 0; n < 3; n++) {
      int field_block = field[m][n + 4];
      int piece_block = piece_shape[m][n];

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

  //float delta_time = (SDL_GetTicks() - last_frame_time) / 1000.0f;
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

  if (has_piece) {
    for (int i = 0; i < 4; i++) {
      int *tile = curr_piece[i];
      int row_idx = tile[0];
      int col_idx = tile[1];

      render_field_block(row_idx, col_idx);
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
