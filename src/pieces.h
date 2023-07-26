#ifndef _PIECES
#define _PIECES

typedef enum {
  T,
  L,
  J,
  S,
  Z,
  O,
  I,
  NUM_PIECE_TYPES,
} PieceType;

struct Piece {
  int shape[4][4];
  int shape_size;

  // the rotation offsets contain the offsets for every tile
  // when rotating counter clockwise according to
  // https://tetris.wiki/Original_Rotation_System
  // the indexes match the tile indexes in curr_piece (row-major traversing)
  int rotation_offsets[4][4][2];
  PieceType type;

  // in order to center all pieces in the preview, we need the bounding box of
  // of the shape within the 4x4 grid in the first position; the first index is the max width
  // and the second the max height
};

const static struct Piece piece_t = {
  .shape = {
    {0, 0, 0, 0},
    {1, 1, 1, 0},
    {0, 1, 0, 0},
    {0, 0, 0, 0},
  },
  .rotation_offsets = {
    { { 1,  1}, {0, 0}, {-1, -1}, {-1,  1} },
    { {-1,  1}, {0, 0}, { 1, -1}, {-1, -1} },
    { {-1, -1}, {0, 0}, { 1,  1}, { 1, -1} },
    { { 1, -1}, {0, 0}, {-1,  1}, { 1,  1} },
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

const static struct Piece piece_j = {
  .shape = {
    {0, 0, 0, 0},
    {1, 1, 1, 0},
    {0, 0, 1, 0},
    {0, 0, 0, 0},
  },
  .rotation_offsets = {
    { {1, 1}, {0, 0}, {-1, -1}, {-2, 0} },
    { {-1, 1}, {0, 0}, {1, -1}, {0, -2} },
    { {-1, -1}, {0, 0}, {1, 1}, {2, 0} },
    { {1, -1}, {0, 0}, {-1, 1}, {0, 2} },
  },
  .type = J,
};

const static struct Piece piece_s = {
  .shape = {
    {0, 0, 0, 0},
    {0, 1, 1, 0},
    {1, 1, 0, 0},
    {0, 0, 0, 0},
  },
  .rotation_offsets = {
    { {0, 0}, {-1, -1}, {0, 2}, {-1, 1} },
    { {0, 0}, {1, 1}, {0, -2}, {1, -1} },
    { {0, 0}, {-1, -1}, {0, 2}, {-1, 1} },
    { {0, 0}, {1, 1}, {0, -2}, {1, -1} },
  },
  .type = S,
};

const static struct Piece piece_z = {
  .shape = {
    {0, 0, 0, 0},
    {1, 1, 0, 0},
    {0, 1, 1, 0},
    {0, 0, 0, 0},
  },
  .rotation_offsets = {
    { {1, 1}, {0, 0}, {-1, 1}, {-2, 0} },
    { {-1, -1}, {0, 0}, {1, -1}, {2, 0} },
    { {1, 1}, {0, 0}, {-1, 1}, {-2, 0} },
    { {-1, -1}, {0, 0}, {1, -1}, {2, 0} },
  },
  .type = Z,
};

const static struct Piece piece_o = {
  .shape = {
    {0, 0, 0, 0},
    {0, 1, 1, 0},
    {0, 1, 1, 0},
    {0, 0, 0, 0},
  },
  .rotation_offsets = {
    { {0, 0}, {0, 0}, {0, 0}, {0, 0} },
    { {0, 0}, {0, 0}, {0, 0}, {0, 0} },
    { {0, 0}, {0, 0}, {0, 0}, {0, 0} },
    { {0, 0}, {0, 0}, {0, 0}, {0, 0} },
  },
  .type = O,
};

const static struct Piece piece_i = {
  .shape = {
    {0, 0, 0, 0},
    {1, 1, 1, 1},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
  },
  .rotation_offsets = {
    { {2, 2}, {1, 1}, {0, 0}, {-1, -1} },
    { {-2, -2}, {-1, -1}, {0, 0}, {1, 1} },
    { {2, 2}, {1, 1}, {0, 0}, {-1, -1} },
    { {-2, -2}, {-1, -1}, {0, 0}, {1, 1} },
  },
  .type = I,
};

#endif

const struct Piece* get_piece(PieceType piece_type);
const int (*get_shape(PieceType piece_type))[4];
const int (*get_rotation_offsets(PieceType piece_type))[4][2];
