#include <gmp.h>
#include <inttypes.h>

#include "chess.h"

#define min(a, b) ((a) < (b) ? (a) : (b))

#define FOUR_FACTORIAL (4 * 3 * 2)

#define ENPASSANT_ONE_ADJACENT_VARIATIONS 2 * (BOARD_SIDE_LENGTH - 2)
#define ENPASSANT_EDGE_VARIATIONS 2
#define ENPASSANT_TWO_ADJACENT_VARIATIONS (BOARD_SIDE_LENGTH - 2)
#define ONE_FIXED_ROOK_VARIATIONS 2

// Can have 0, 1, or 2 rooks with castling rights
// |{0, 1, 2}| = 3
char BASE_PIECES[3][NUM_PIECE_TYPES_LESS_KING] = {
    {2, 2, 2, 1}, {2, 2, 1, 1}, {2, 2, 1, 0}};
//// b, n, r, q    b, n, r, q    b, n, q, r
////////////////////////////////////// <-->
/// Notice how we switch queens and rooks ^

extern char permutationsOf0to3[24][NUM_PIECE_TYPES_LESS_KING];

typedef struct position_node {
  // const?
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
};

void map_permutation(char input[NUM_PIECE_TYPES_LESS_KING],
                     char mapping[NUM_PIECE_TYPES_LESS_KING],
                     char result[NUM_PIECE_TYPES_LESS_KING]);

char num_promotions(char pieces[NUM_PIECE_TYPES_LESS_KING],
                    char base_piece_limits[NUM_PIECE_TYPES_LESS_KING]) {
  char promotions = 0;
  for (int i = 0; i < NUM_PIECE_TYPES_LESS_KING; i++) {
    if (pieces[i] > base_piece_limits[i]) {
      promotions += pieces[i] - base_piece_limits[i];
    }
  }

  return promotions;
}
