#include <inttypes.h>
#include <stdbool.h>

#include "chess.h"

#ifndef POSITION
#define POSITION

typedef struct piece {
  char type;
  uint64_t bitboard;
} piece;

typedef struct side {
  uint64_t pieces[NUM_PIECE_TYPES];
  uint64_t pawns;
  uint64_t fixed_rooks;
} side;

typedef struct position {
  side sides[NUM_SIDES];
  uint64_t enpassant;
  bool side0isBlack;
} position;

#endif
