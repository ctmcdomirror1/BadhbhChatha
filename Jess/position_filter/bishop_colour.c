#include <nmmintrin.h>

#include "bishop_colour.h"
#include "chess.h"
#include "position.h"
#include "util.h"

char validate_num_promotions(position p) {
  char base_piece_limits_pside[NUM_SIDES][NUM_PIECE_TYPES_LESS_KING];
  char num_pieces[NUM_SIDES][NUM_PIECE_TYPES_LESS_KING];
  char total_pieces[NUM_SIDES] = {0};
  char num_pawns[NUM_SIDES] = {0};
  char total_promotions = 0;
  for (char i = 0; i < NUM_SIDES; i++) {
    for (char j = 0; j < NUM_PIECE_TYPES_LESS_KING; j++) {
      base_piece_limits_pside[i][j] = BASE_PIECE_LIMITS[j];
      num_pieces[i][j] = _mm_popcnt_u64(p.sides[i].pieces[j]);
      total_pieces[i] += num_pieces[i][j];
    }
    base_piece_limits_pside[i][BISHOP] =
        1; // overwrite bishop's value and treat as light square bishops
    num_pieces[i][BISHOP] =
        _mm_popcnt_u64(p.sides[i].pieces[BISHOP] & WHITE_SQ_MASK);

    // and count dark square bishops separately
    char min_proms = num_promotions(num_pieces[i], base_piece_limits_pside[i]);
    char num_black_sq_bishops =
        _mm_popcnt_u64(p.sides[i].pieces[BISHOP] & BLACK_SQ_MASK);
    if (num_black_sq_bishops > 1) {
      min_proms += (num_black_sq_bishops - 1);
    }

    num_pawns[i] += _mm_popcnt_u64(p.sides[i].pawns);
    if (min_proms > (NUM_PAWNS_PSIDE - num_pawns[i])) {
      return SIDE0_MORE_PROMOTIONS_THAN_MISSING_PAWNS + i;
    }
    total_promotions += min_proms;
  }

  for (int i = 0; i < NUM_SIDES; i++) {
    if (total_promotions > (2 * (NUM_PAWNS_PSIDE - num_pawns[i]) +
                            (NUM_PIECES_PSIDE_LESS_KING - total_pieces[i]))) {
      return SIDE0_ALLOWS_LESS_THAN_TOTAL_PROMOTIONS + i;
    }
  }

  return 0;
}
