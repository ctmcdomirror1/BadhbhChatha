#include <assert.h>
#include <nmmintrin.h>
#include <stdlib.h>

#include "position.h"
#include "util.h"

void sanity_check(position p) {

  // no two chessmen on same square
  uint64_t checkers = 0, chessmen_sum = 0;
  for (int i = 0; i < 2; i++) {
    checkers += p.sides[i].pawns;
    chessmen_sum += _mm_popcnt_u64(p.sides[i].pawns);
    for (int j = 0; j < 5; j++) {
      checkers += p.sides[i].pieces[j];
      chessmen_sum += _mm_popcnt_u64(p.sides[i].pieces[j]);
    }
  }
  assert(chessmen_sum == _mm_popcnt_u64(checkers));

  // only one king per side
  for (int i = 0; i < 2; i++) {
    assert(1 == _mm_popcnt_u64(p.sides[i].pieces[4]));
  }

  // pawns aren't on first or last rows
  for (int i = 0; i < 2; i++) {
    if (p.sides[i].pawns != 0) {
      assert((p.sides[i].pawns >= rcb(1, 0)) && (p.sides[i].pawns < rcb(7, 0)));
    }
  }

  // enpassant
  int num_enpassant_pawns = _mm_popcnt_u64(p.enpassant);
  if (num_enpassant_pawns == 1) {
    // only on 4th rank
    assert(p.enpassant >= rcb(3, 0) && p.enpassant < rcb(3 + 1, 0));
    // and nothing behind it
    assert(0 == (checkers & (p.enpassant >> 8)));
    assert(0 == (checkers & (p.enpassant >> (2 * 8))));
    // at least one adjacent pawn
    uint64_t adjacent_mask;
    if (rcb(3, 0) == p.enpassant) {
      adjacent_mask = rcb(3, 1);
    } else if (rcb(3, 7) == p.enpassant) {
      adjacent_mask = rcb(3, 6);
    } else {
      adjacent_mask = (p.enpassant << 1) + (p.enpassant >> 1);
    }
    assert(p.sides[1].pawns & adjacent_mask);
    // is accounted for by pawns
    assert(p.enpassant & p.sides[0].pawns);
  } else {
    assert(0 == num_enpassant_pawns);
  }

  // rooks with castling rights
  // and kings at home
  if (0 < p.sides[0].fixed_rooks) {
    uint64_t fixed_rooks_mask_compliment = ~(rcb(0, 0) + rcb(0, 7));
    assert(0 == (p.sides[0].fixed_rooks & fixed_rooks_mask_compliment));
    assert(0 != (p.sides[0].pieces[4] & rcb(0, 3)));
    // is accounted for by rooks
    assert(p.sides[0].fixed_rooks & p.sides[0].pieces[2]);
  }
  if (0 < p.sides[1].fixed_rooks) {
    uint64_t fixed_rooks_mask_compliment = ~(rcb(7, 0) + rcb(7, 7));
    assert(0 == (p.sides[1].fixed_rooks & fixed_rooks_mask_compliment));
    assert(0 != (p.sides[1].pieces[4] & rcb(7, 3)));
    assert(p.sides[1].fixed_rooks & p.sides[1].pieces[2]);
  }

  // promotions
  char base_piece_limits_pside[2][4];
  char num_pieces[2][4] = {0};
  char total_pieces[2] = {0};
  char num_pawns[2] = {0};
  char total_promotions = 0;
  for (char i = 0; i < 2; i++) {
    for (char j = 0; j < 4; j++) {
      base_piece_limits_pside[i][j] = BASE_PIECE_LIMITS[j];
      num_pieces[i][j] = _mm_popcnt_u64(p.sides[i].pieces[j]);
      total_pieces[i] += num_pieces[i][j];
    }

    num_pawns[i] += _mm_popcnt_u64(p.sides[i].pawns);
    char min_proms = num_promotions(num_pieces[i], base_piece_limits_pside[i]);
    assert(min_proms <= (8 - num_pawns[i]));
    total_promotions += min_proms;
  }

  for (int i = 0; i < 2; i++) {
    assert(total_promotions <=
           (2 * (8 - num_pawns[i]) + (7 - total_pieces[i])));
  }
}
