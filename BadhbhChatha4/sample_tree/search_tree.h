#include <assert.h>
#include <gmp.h>
#include <immintrin.h>
#include <inttypes.h>
#include <nmmintrin.h>
#include <stdio.h>

#include "position.h"
#include "tree_common.h"
#include "util.h"

#define EDGE_ROW_MASK                                                          \
  -1 - ((1UL << (7 * BOARD_SIDE_LENGTH)) - 1) + ((1UL << BOARD_SIDE_LENGTH) - 1)

char base_piece_limits_pside[NUM_SIDES][NUM_PIECE_TYPES_LESS_KING];

char point_root_to_matching_child(position_node **root, mpz_t index) {
  char i = 0;
  for (; mpz_cmp(index, (*root)->children[i]->num_positions) >= 0; i++) {
    mpz_sub(index, index, // if rng > positions in subtree
            (*root)->children[i]->num_positions); // then crashing is acceptable
  }                                               // because there's a bug
  assert(i < (*root)->num_children);

  *root = (*root)->children[i];

  return i;
}

uint64_t pass_enpassant(position_node **root, mpz_t index, position *p,
                        uint64_t *occupied_squares) {
  mpz_t rem;
  mpz_init(rem);
  char remui;
  uint64_t unoccupiable_adjacent = 0;

  char enpassant_case = point_root_to_matching_child(root, index);
  if (enpassant_case == ENPASSANT_ONE_ADJACENT) {
    mpz_fdiv_qr_ui(index, rem, index, ENPASSANT_ONE_ADJACENT_VARIATIONS);
    remui = mpz_get_ui(rem);

    p->enpassant =
        1UL << (ENPASSANT_ROW_0INDEX * BOARD_SIDE_LENGTH + 1 + (remui / 2));
    if (remui % 2) {
      p->sides[1].pawns = p->enpassant << 1;
      unoccupiable_adjacent = p->enpassant >> 1;
    } else {
      unoccupiable_adjacent = p->enpassant << 1;
      p->sides[1].pawns = p->enpassant >> 1;
    }

  } else if (enpassant_case == ENPASSANT_EDGE) {
    mpz_fdiv_qr_ui(index, rem, index, ENPASSANT_EDGE_VARIATIONS);
    remui = mpz_get_ui(rem);
    if (remui == 0) {
      p->enpassant = 1UL << (ENPASSANT_ROW_0INDEX * BOARD_SIDE_LENGTH);
      p->sides[1].pawns = 1UL << (ENPASSANT_ROW_0INDEX * BOARD_SIDE_LENGTH + 1);
    } else {
      p->enpassant =
          1UL << ((ENPASSANT_ROW_0INDEX + 1) * BOARD_SIDE_LENGTH - 1);
      p->sides[1].pawns =
          1UL << ((ENPASSANT_ROW_0INDEX + 1) * BOARD_SIDE_LENGTH - 2);
    }

  } else if (enpassant_case == ENPASSANT_TWO_ADJACENT) {
    mpz_fdiv_qr_ui(index, rem, index, ENPASSANT_TWO_ADJACENT_VARIATIONS);
    remui = mpz_get_ui(rem);
    p->enpassant =
        1UL << (ENPASSANT_ROW_0INDEX * BOARD_SIDE_LENGTH + 1 + (remui / 2));
    p->sides[1].pawns = (p->enpassant << 1) + (p->enpassant >> 1);
  } else if (enpassant_case != NO_ENPASSANT) {
    assert(false);
  }

  p->sides[0].pawns = p->enpassant;
  *occupied_squares =
      p->sides[0].pawns + p->sides[1].pawns + unoccupiable_adjacent;
  if (enpassant_case != NO_ENPASSANT) {
    *occupied_squares += ((*p).enpassant >> BOARD_SIDE_LENGTH) +
                         ((*p).enpassant >> (2 * BOARD_SIDE_LENGTH));
  }

  mpz_clear(rem);

  return unoccupiable_adjacent;
}

uint64_t place_chessmen_relative_to_free_squares(char num_chessmen,
                                                 char num_free_squares,
                                                 uint64_t index) {
  if (num_chessmen == 0) {
    return 0;
  }

  for (int i = 0; i < num_free_squares - (num_chessmen - 1); i++) {
    if (index < binomials[num_free_squares - 1 - i][num_chessmen - 1]) {
      uint64_t further_chessmen =
          place_chessmen_relative_to_free_squares(
              num_chessmen - 1, num_free_squares - 1 - i, index)
          << (i + 1);
      return (1UL << i) + further_chessmen;
    }

    index -= binomials[num_free_squares - 1 - i][num_chessmen - 1];
  }
  assert(false);
}

char pass_generic(position_node **root, mpz_t index, uint64_t *occupied_squares,
                  uint64_t *bitboard) {
  char num_chessmen = point_root_to_matching_child(root, index);
  if (num_chessmen == 0) {
    return 0;
  }

  mpz_t rem;
  mpz_init(rem);

  char num_free_squares = NUM_SQUARES - _mm_popcnt_u64(*occupied_squares);
  mpz_fdiv_qr_ui(index, rem, index, binomials[num_free_squares][num_chessmen]);
  uint64_t chessmen =
      _pdep_u64(place_chessmen_relative_to_free_squares(
                    num_chessmen, num_free_squares, mpz_get_ui(rem)),
                ~(*occupied_squares));
  *bitboard += chessmen;
  *occupied_squares += chessmen;

  mpz_clear(rem);

  return num_chessmen;
}

bool pass_fixed_rooks_and_kings(position_node **root, mpz_t index, position *p,
                                uint64_t *occupied_squares, bool side) {
  mpz_t rem;
  mpz_init(rem);
  uint64_t remui;

  uint64_t king = rcb(0, KING_HOME_COLUMN_0INDEX);
  uint64_t fixed_rooks = 0;

  char num_fixed_rooks = point_root_to_matching_child(root, index);
  if (num_fixed_rooks == 1) {
    mpz_fdiv_qr_ui(index, rem, index, ONE_FIXED_ROOK_VARIATIONS);
    remui = mpz_get_ui(rem);
    fixed_rooks = 1UL << ((BOARD_SIDE_LENGTH - 1) * remui);

  } else if (num_fixed_rooks == 2) {
    fixed_rooks = 1 + (1UL << (BOARD_SIDE_LENGTH - 1));

  } else if (num_fixed_rooks == 0) {
    mpz_clear(rem);
    return false;
  } else {
    assert(false);
  }

  if (num_fixed_rooks > 0 && side == 1) {
    king = rotate_bitboard_across_central_rows(king);
    fixed_rooks = rotate_bitboard_across_central_rows(fixed_rooks);
  }

  p->sides[side].fixed_rooks = fixed_rooks;
  p->sides[side].pieces[KING] = king;
  *occupied_squares += fixed_rooks + king;

  mpz_clear(rem);

  return true;
}

// This could be a lot more efficient but we don't need it to be.
// ^ That comment was based on the plan to compute a lowerbound. If just
// computing an upperbound it would be cool to make this a lot more efficient,
// same for permutation calculations in tree creation
void get_indexed_permutation_helper(
    char num_pieces_permutation[NUM_SIDES][NUM_PIECE_TYPES_LESS_KING], char i,
    char j, char num_pawns[NUM_SIDES], char max_proms, short permutation_index,
    char (*indexed_permutation)[NUM_SIDES][NUM_PIECE_TYPES_LESS_KING],
    short *permutation_count) {

  if (j == (NUM_PIECE_TYPES_LESS_KING - 1)) {
    if (i == 0) {
      i = 1;
      j = 0;
    } else {
      char prom0 =
          num_promotions(num_pieces_permutation[0], base_piece_limits_pside[0]);
      if (prom0 > (NUM_PAWNS_PSIDE - num_pawns[0])) {
        return;
      }
      char prom1 =
          num_promotions(num_pieces_permutation[1], base_piece_limits_pside[1]);
      if (prom1 > (NUM_PAWNS_PSIDE - num_pawns[1])) {
        return;
      }
      if ((prom0 + prom1) > max_proms) {
        return;
      }

      if (*permutation_count == permutation_index) {
        for (int k = 0; k < NUM_SIDES; k++) {
          for (int l = 0; l < NUM_PIECE_TYPES_LESS_KING; l++) {
            (*indexed_permutation)[k][l] = num_pieces_permutation[k][l];
          }
        }
      }

      (*permutation_count)++;
      return;
    }
  }

  char prev = -1;
  for (int k = j; k < NUM_PIECE_TYPES_LESS_KING; k++) {
    char staged_permutation[NUM_SIDES][NUM_PIECE_TYPES_LESS_KING];
    for (int l = 0; l < NUM_SIDES; l++) {
      for (int m = 0; m < NUM_PIECE_TYPES_LESS_KING; m++) {
        staged_permutation[l][m] = num_pieces_permutation[l][m];
      }
    }

    if (k > j && staged_permutation[i][j] == staged_permutation[i][k]) {
      continue;
    }
    if (prev != -1 && prev == num_pieces_permutation[i][k]) {
      continue;
    }

    char tmp = staged_permutation[i][j];
    staged_permutation[i][j] = staged_permutation[i][k];
    staged_permutation[i][k] = tmp;

    prev = num_pieces_permutation[i][k];

    get_indexed_permutation_helper(staged_permutation, i, j + 1, num_pawns,
                                   max_proms, permutation_index,
                                   indexed_permutation, permutation_count);
  }
}

short get_indexed_permutation(
    char num_pieces_permutation[NUM_SIDES][NUM_PIECE_TYPES_LESS_KING],
    char num_pawns[NUM_SIDES], char max_proms, uint64_t permutation_index,
    char (*indexed_permutation)[NUM_SIDES][NUM_PIECE_TYPES_LESS_KING]) {
  short count = 0;
  get_indexed_permutation_helper(num_pieces_permutation, 0, 0, num_pawns,
                                 max_proms, permutation_index,
                                 indexed_permutation, &count);
  return count;
}

// Return a chessboard indexed by the GMP mpz_t 'index'
position retrieve_position(position_node *root, mpz_t index) {
  position p = {0};

  uint64_t occupied_squares = 0;
  // tmp_unoccupiable_adjacent is used to block the currently empty adjacent
  // in the case of ENPASSANT_ONE_ADJACENT
  uint64_t tmp_unoccupiable_enpassant_adjacent =
      pass_enpassant(&root, index, &p, &occupied_squares);

  // Pawns can't be placed on edge rows
  occupied_squares += EDGE_ROW_MASK;
  pass_generic(&root, index, &occupied_squares, &p.sides[1].pawns);

  occupied_squares -= tmp_unoccupiable_enpassant_adjacent;
  pass_generic(&root, index, &occupied_squares, &p.sides[0].pawns);
  occupied_squares -= EDGE_ROW_MASK;
  bool fixed_kings[NUM_SIDES];
  fixed_kings[0] =
      pass_fixed_rooks_and_kings(&root, index, &p, &occupied_squares, 0);
  fixed_kings[1] =
      pass_fixed_rooks_and_kings(&root, index, &p, &occupied_squares, 1);

  char num_chessmen;
  for (int i = 0; i < NUM_SIDES; i++) {
    for (int j = 0; j < NUM_PIECE_TYPES_LESS_KING; j++) {
      num_chessmen =
          pass_generic(&root, index, &occupied_squares, &p.sides[i].pieces[j]);
      if (num_chessmen == 0) {
        break;
      }
    }
  }
  mpz_t rem;
  mpz_init(rem);
  char num_free_squares = NUM_SQUARES - _mm_popcnt_u64(occupied_squares);
  for (int i = 0; i < NUM_SIDES; i++) {
    if (!fixed_kings[i]) {
      mpz_fdiv_qr_ui(index, rem, index, num_free_squares);
      p.sides[i].pieces[KING] =
          _pdep_u64(place_chessmen_relative_to_free_squares(1, num_free_squares,
                                                            mpz_get_ui(rem)),
                    ~(occupied_squares));
      occupied_squares += p.sides[i].pieces[KING];
      num_free_squares--;
    }
  }
  mpz_clear(rem);

  char num_capturable_pieces[NUM_SIDES][NUM_PIECE_TYPES_LESS_KING];
  char total_capturable_pieces[NUM_SIDES];
  char num_pawns[NUM_SIDES];
  char total_promotions = 0;
  char prom_lims[NUM_SIDES];
  for (int i = 0; i < NUM_SIDES; i++) {
    char nfr = _mm_popcnt_u64(p.sides[i].fixed_rooks);
    total_capturable_pieces[i] = nfr;
    for (int j = 0; j < NUM_PIECE_TYPES_LESS_KING; j++) {
      base_piece_limits_pside[i][j] = BASE_PIECE_LIMITS[j];
      num_capturable_pieces[i][j] = _mm_popcnt_u64(p.sides[i].pieces[j]);
      total_capturable_pieces[i] += num_capturable_pieces[i][j];
    }
    base_piece_limits_pside[i][ROOK] -= nfr;

    num_pawns[i] = _mm_popcnt_u64(p.sides[i].pawns);

    prom_lims[i] = 2 * (NUM_PAWNS_PSIDE - num_pawns[i]) +
                   (NUM_PIECES_PSIDE_LESS_KING - total_capturable_pieces[i]);
  }
  char max_proms = min(prom_lims[0], prom_lims[1]);

  char permutation[NUM_SIDES][NUM_PIECE_TYPES_LESS_KING] = {0};
  short num_permutations =
      get_indexed_permutation(num_capturable_pieces, num_pawns, max_proms,
                              mpz_get_ui(index), &permutation);

  uint64_t expected_num_positions = num_permutations;
  for (int i = 0; i < NUM_SIDES; i++) {
    if (!fixed_kings[i]) {
      num_free_squares++;
      expected_num_positions *= num_free_squares;
    }
  }
  expected_num_positions *=
      binomials[num_free_squares + num_chessmen][num_chessmen];
  assert(expected_num_positions == mpz_get_ui(root->num_positions));

  for (int i = 0; i < NUM_SIDES; i++) {
    bool accounted_for[NUM_PIECE_TYPES_LESS_KING] = {0};
    uint64_t tmp[NUM_PIECE_TYPES_LESS_KING];
    for (int j = 0; j < NUM_PIECE_TYPES_LESS_KING; j++) {
      bool found = false;
      for (int k = 0; k < NUM_PIECE_TYPES_LESS_KING; k++) {
        if (accounted_for[k]) {
          continue;
        }

        if (permutation[i][j] == num_capturable_pieces[i][k]) {
          tmp[j] = p.sides[i].pieces[k];
          accounted_for[k] = true;
          found = true;
          break;
        }
      }
      assert(found);
    }

    for (int j = 0; j < NUM_PIECE_TYPES_LESS_KING; j++) {
      p.sides[i].pieces[j] = tmp[j];
    }
    p.sides[i].pieces[ROOK] += p.sides[i].fixed_rooks;
  }

  return p;
}
