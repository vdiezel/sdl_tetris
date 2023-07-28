#include "./pieces.h"

// my poor man's polymorphism
const struct Piece* get_piece(PieceType piece_type) {

  switch (piece_type) {
    case T:
      return &piece_t;
    case L:
      return &piece_l;
    case J:
      return &piece_j;
    case S:
      return &piece_s;
    case Z:
      return &piece_z;
    case O:
      return &piece_o;
    case I:
      return &piece_i;
    default:
      return &piece_t;
  }

}

const int (*get_shape(PieceType piece_type))[4] {
  const struct Piece* piece = get_piece(piece_type);
  return piece->shape;
}

const int (*get_rotation_offsets(PieceType piece_type))[4][2] {
  const struct Piece* piece = get_piece(piece_type);
  return piece->rotation_offsets;
}

const int *get_shape_size(PieceType piece_type) {
  const struct Piece* piece = get_piece(piece_type);
  return piece->shape_size;
}
