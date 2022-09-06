#pragma once
#include <gmp.h>
#include <inttypes.h>

#include "chess.h"

#define min(a, b) ((a) < (b) ? (a) : (b))

#define FOUR_FACTORIAL (4 * 3 * 2)

#define ENPASSANT_ONE_ADJACENT_VARIATIONS 2 * (BOARD_SIDE_LENGTH - 2)
#define ENPASSANT_EDGE_VARIATIONS 2
#define ENPASSANT_TWO_ADJACENT_VARIATIONS (BOARD_SIDE_LENGTH - 2)
#define ONE_FIXED_ROOK_VARIATIONS 2

extern char permutationsOf0to3[24][NUM_PIECE_TYPES_LESS_KING];

typedef struct position_node {
  mpz_t num_positions;
  struct position_node **children;
  char num_children;
} position_node;

enum enpassant_cases {
  NO_ENPASSANT,
  ENPASSANT_ONE_ADJACENT,
  ENPASSANT_EDGE,
  ENPASSANT_TWO_ADJACENT
};

enum castling_rights_cases {
  NO_CASTLING_RIGHTS,
  CASTLING_RIGHTS_ONE_SIDE,
  CASTLING_RIGHTS_BOTH_SIDES
}

void map_permutation(char input[NUM_PIECE_TYPES_LESS_KING],
                     char mapping[NUM_PIECE_TYPES_LESS_KING],
                     char result[NUM_PIECE_TYPES_LESS_KING]);
