#include <stdio.h>
#include <SDL2/SDL.h>
#include "./constants.h"

int CAP_FRAME_RATE = TRUE;

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

int game_is_running = FALSE;
int last_frame_time = 0;
int field[FIELD_ROWS][FIELD_COLUMNS];

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

void handle_key_down(SDL_Event event) {
  switch (event.key.keysym.sym) {
    case SDLK_ESCAPE:
      game_is_running = FALSE;
      break;
    case SDLK_1:
      field[19][0] = 1;
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

  //float delta_time = (SDL_GetTicks() - last_frame_time) / 1000.0f;
  last_frame_time = SDL_GetTicks();
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
        SDL_Rect block = {
          FIELD_ORIGIN_X + i * BLOCK_WIDTH,
          FIELD_ORIGIN_Y + j * BLOCK_WIDTH,
          BLOCK_WIDTH,
          BLOCK_WIDTH,
        };
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(renderer, &block);
      }
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
