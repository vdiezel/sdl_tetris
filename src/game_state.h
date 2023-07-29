#ifndef _GAME_STATE
#define _GAME_STATE

#include "./pieces.h"
#include "./constants.h"

typedef struct GameState {
  int game_is_running;
  int has_piece;
  int last_frame_time;
  int field[FIELD_ROWS][FIELD_COLUMNS];

  // this holds the field indexes if the tiles of the current piece
  int curr_piece[4][2];
  PieceType piece_type;
  int curr_rot;
  PieceType next_piece_type;

  // stores the timestamp when the last application
  // of gravity happened
  int last_gravity;
} GameState;

#endif
