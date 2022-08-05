#include <assert.h>
#include <gmp.h>
#include <stdbool.h>
#include <stdlib.h>

#include "chess.h"
#include "tree_common.h"
#include "util.h"

#define min3(a, b, c) min(min(a, b), c)

char factorials[NUM_PIECE_TYPES_LESS_KING] = {1, 2, 6, FOUR_FACTORIAL};
char pieces[NUM_SIDES][NUM_PIECE_TYPES_LESS_KING] = {0};

typedef struct position_state {
  char free_squares;
  char fixed_pawns[NUM_SIDES];
  bool fixed_kings[NUM_SIDES];
  char pawns[NUM_SIDES];
  char total_pieces[NUM_SIDES];
  char base_piece_limits_pside[NUM_SIDES][NUM_PIECE_TYPES_LESS_KING];
} position_state;

typedef struct two_entry_wrapped_char_array {
  char A[NUM_SIDES];
} two_entry_wrapped_char_array;

void map_permutation(char input[NUM_PIECE_TYPES_LESS_KING],
                     char mapping[NUM_PIECE_TYPES_LESS_KING],
                     char permutation[NUM_PIECE_TYPES_LESS_KING]) {
  for (int i = 0; i < NUM_PIECE_TYPES_LESS_KING; i++) {
    permutation[i] = input[mapping[i]];
  }
}

// We use precomputed permutations here but not in the search tree in an
// effort to spot bugs in the search tree implementation.
char permutationsOf0to3[][NUM_PIECE_TYPES_LESS_KING] = {
    {0, 1, 2, 3}, {0, 1, 3, 2}, {0, 3, 1, 2}, {3, 0, 1, 2}, {0, 2, 1, 3},
    {0, 2, 3, 1}, {0, 3, 2, 1}, {3, 0, 2, 1}, {2, 0, 1, 3}, {2, 0, 3, 1},
    {2, 3, 0, 1}, {3, 2, 0, 1}, {1, 0, 2, 3}, {1, 0, 3, 2}, {1, 3, 0, 2},
    {3, 1, 0, 2}, {1, 2, 0, 3}, {1, 2, 3, 0}, {1, 3, 2, 0}, {3, 1, 2, 0},
    {2, 1, 0, 3}, {2, 1, 3, 0}, {2, 3, 1, 0}, {3, 2, 1, 0}};

// This could be a lot more efficient but we don't need it to be.
short num_piece_type_permutations(
    char base_piece_limits_pside[NUM_SIDES][NUM_PIECE_TYPES_LESS_KING],
    char pawns[NUM_SIDES],
    char num_pieces[NUM_SIDES][NUM_PIECE_TYPES_LESS_KING],
    char total_pieces[NUM_SIDES]) {

  char prom_lim1 = NUM_PAWNS_PSIDE - pawns[0];
  char prom_lim2 = NUM_PAWNS_PSIDE - pawns[1];
  char prom_lim3 =
      2 * prom_lim1 + (NUM_PIECES_PSIDE_LESS_KING - total_pieces[0]);
  char prom_lim4 =
      2 * prom_lim2 + (NUM_PIECES_PSIDE_LESS_KING - total_pieces[1]);
  char prom_sum_lim = min(prom_lim3, prom_lim4);

  char permutation[NUM_PIECE_TYPES_LESS_KING];
  short legal_permutations = 0;
  for (int i = 0; i < FOUR_FACTORIAL; i++) {
    map_permutation(num_pieces[0], permutationsOf0to3[i], permutation);
    char np0 = num_promotions(permutation, base_piece_limits_pside[0]);
    if (np0 > prom_lim1) {
      continue;
    }

    for (int j = 0; j < FOUR_FACTORIAL; j++) {
      map_permutation(num_pieces[1], permutationsOf0to3[j], permutation);
      char np1 = num_promotions(permutation, base_piece_limits_pside[1]);
      if (np1 > prom_lim2) {
        continue;
      }

      if ((np0 + np1) <= prom_sum_lim) {
        legal_permutations++;
      }
    }
  }
  assert(legal_permutations > 0);

  short factors_product = 1;
  for (int i = 0; i < NUM_SIDES; i++) {
    char streak = 1;
    for (int j = 1; j < NUM_PIECE_TYPES_LESS_KING; j++) {
      if (num_pieces[i][j] == num_pieces[i][j - 1]) {
        streak++;
      } else {
        factors_product *= factorials[streak - 1];
        streak = 1;
      }
    }
    factors_product *= factorials[streak - 1];
  }
  assert((legal_permutations % factors_product) == 0);
  legal_permutations /= factors_product;

  return legal_permutations;
}

void count_from_pieces_helper(
    const char free_squares, const bool fixed_kings[NUM_SIDES],
    char pawns[NUM_SIDES],
    char base_piece_limits_pside[NUM_SIDES][NUM_PIECE_TYPES_LESS_KING],
    char num_pieces[NUM_SIDES][NUM_PIECE_TYPES_LESS_KING],
    two_entry_wrapped_char_array total_pieces,
    two_entry_wrapped_char_array promotions, position_node *root,
    const char rel_level, two_entry_wrapped_char_array previous_pieces) {
  mpz_init(root->num_positions);

  const bool side = rel_level >= NUM_PIECE_TYPES_LESS_KING;
  const char piece_type = (int)(rel_level % NUM_PIECE_TYPES_LESS_KING);
  char new_base_pieces_limit = base_piece_limits_pside[side][piece_type];

  char new_pieces_lim1 = new_base_pieces_limit +
                         2 * (NUM_PAWNS_PSIDE - pawns[!side]) +
                         (NUM_PIECES_PSIDE_LESS_KING - total_pieces.A[!side]) -
                         promotions.A[!side] - promotions.A[side];
  char new_pieces_lim2 = 2 * (NUM_PAWNS_PSIDE - pawns[side]) +
                         (NUM_PIECES_PSIDE_LESS_KING - total_pieces.A[side]) -
                         promotions.A[!side] - promotions.A[side];
  if (new_pieces_lim2 > new_base_pieces_limit) {
    new_pieces_lim2 = (char)((new_pieces_lim2 - new_base_pieces_limit) / 2);
  }
  char new_pieces_lim3 = new_base_pieces_limit + NUM_PAWNS_PSIDE - pawns[side] -
                         promotions.A[side];
  char new_pieces_lim = min3(new_pieces_lim1, new_pieces_lim2, new_pieces_lim3);
  assert(0 <= new_pieces_lim && new_pieces_lim <= 10);

  root->num_children = min(1 + new_pieces_lim, 1 + previous_pieces.A[side]);
  assert(0 <= root->num_children && root->num_children <= 11);
  root->children =
      (position_node **)malloc(root->num_children * sizeof(position_node *));
  for (char i = 0; i < root->num_children; i++) {
    root->children[i] = (position_node *)malloc(sizeof(position_node));
  }

  for (int i = 0; i < root->num_children; i++) {
    assert(0 <= free_squares - i && free_squares - i <= 64);
    assert(total_pieces.A[side] + i <= 15);
    char new_promotions_of_this = 0;
    if (i > base_piece_limits_pside[side][piece_type]) {
      new_promotions_of_this = i - base_piece_limits_pside[side][piece_type];
    }
    assert(0 <= new_promotions_of_this && new_promotions_of_this <= 8);

    num_pieces[side][piece_type] = i;
    if ((side == 1 && i == 0) ||
        rel_level == (2 * NUM_PIECE_TYPES_LESS_KING - 1)) {

      uint64_t variations = binomials[free_squares][i];
      two_entry_wrapped_char_array new_total_pieces;
      new_total_pieces.A[!side] = total_pieces.A[!side];
      new_total_pieces.A[side] = total_pieces.A[side] + i;
      variations *= num_piece_type_permutations(base_piece_limits_pside, pawns,
                                                num_pieces, new_total_pieces.A);

      char kfree_squares = free_squares - i;
      if (!fixed_kings[0]) {
        variations *= kfree_squares;
        kfree_squares--;
      }
      if (!fixed_kings[1]) {
        variations *= kfree_squares;
        kfree_squares--;
      }
      mpz_init_set_ui(root->children[i]->num_positions, variations);
      mpz_add_ui(root->num_positions, root->num_positions, variations);

    } else {
      two_entry_wrapped_char_array new_total_pieces;
      new_total_pieces.A[!side] = total_pieces.A[!side];
      new_total_pieces.A[side] = total_pieces.A[side] + i;

      two_entry_wrapped_char_array new_promotions;
      new_promotions.A[!side] = promotions.A[!side];
      new_promotions.A[side] = promotions.A[side] + new_promotions_of_this;

      two_entry_wrapped_char_array new_previous_pieces;
      new_previous_pieces.A[!side] = previous_pieces.A[!side];
      new_previous_pieces.A[side] = i;

      char new_rel_level = rel_level + 1;
      if (i == 0) {
        new_rel_level = NUM_PIECE_TYPES_LESS_KING;
      }

      count_from_pieces_helper(
          free_squares - i, fixed_kings, pawns, base_piece_limits_pside,
          num_pieces, new_total_pieces, new_promotions, root->children[i],
          new_rel_level, new_previous_pieces);

      mpz_mul_ui(root->children[i]->num_positions,
                 root->children[i]->num_positions, binomials[free_squares][i]);
      mpz_add(root->num_positions, root->num_positions,
              root->children[i]->num_positions);
    }
  }

  num_pieces[side][piece_type] = 0;
}

void count_from_pieces(position_state *s, position_node *root, bool _) {
  char num_pieces[NUM_SIDES][NUM_PIECE_TYPES_LESS_KING] = {0};
  two_entry_wrapped_char_array total_pieces;
  total_pieces.A[0] = s->total_pieces[0];
  total_pieces.A[1] = s->total_pieces[1];

  two_entry_wrapped_char_array previous_pieces;
  previous_pieces.A[0] = MAX_BISHOPS_PSIDE;
  previous_pieces.A[1] = MAX_BISHOPS_PSIDE;

  two_entry_wrapped_char_array promotions = {0};
  assert(0 == promotions.A[0]);
  count_from_pieces_helper(s->free_squares, s->fixed_kings, s->pawns,
                           s->base_piece_limits_pside, num_pieces, total_pieces,
                           promotions, root, 0, previous_pieces);
}

void count_from_fixed_rooks_and_kings(position_state *s, position_node *root,
                                      const bool side) {
  mpz_init(root->num_positions);
  root->num_children = 3;
  root->children =
      (position_node **)malloc(root->num_children * sizeof(position_node *));
  for (char i = 0; i < root->num_children; i++) {
    root->children[i] = (position_node *)malloc(sizeof(position_node));
  }

  typedef void (*fn)(position_state *, position_node *, bool side);
  fn next_call = count_from_pieces;
  if (side == 0) {
    next_call = count_from_fixed_rooks_and_kings;
  }

  for (char i = 0; i < NUM_PIECE_TYPES_LESS_KING; i++) {
    s->base_piece_limits_pside[side][i] = BASE_PIECE_LIMITS[i];
  }

  char free_squares = s->free_squares;
  s->free_squares = free_squares;
  s->total_pieces[side] = 0;
  s->fixed_kings[side] = false;
  next_call(s, root->children[0], !side);
  mpz_add(root->num_positions, root->num_positions,
          root->children[0]->num_positions);

  s->fixed_kings[side] = true;
  s->free_squares = free_squares - 2;
  s->base_piece_limits_pside[side][2]--;
  s->total_pieces[side] = 1;
  next_call(s, root->children[1], !side);
  mpz_mul_ui(root->children[1]->num_positions, root->children[1]->num_positions,
             ONE_FIXED_ROOK_VARIATIONS);
  mpz_add(root->num_positions, root->num_positions,
          root->children[1]->num_positions);

  s->free_squares = free_squares - 3;
  s->base_piece_limits_pside[side][3]--; // swap rooks with queens slot
  s->total_pieces[side] = 2;
  next_call(s, root->children[2], !side);
  mpz_add(root->num_positions, root->num_positions,
          root->children[2]->num_positions);
}

void count_from_pawns(position_state *s, position_node *root, const bool side,
                      const bool unoccupiable_adjacent) {
  mpz_init(root->num_positions);
  root->num_children = 1 + (NUM_PAWNS_PSIDE - s->fixed_pawns[side]);
  root->children =
      (position_node **)malloc(root->num_children * sizeof(position_node *));
  for (char i = 0; i < root->num_children; i++) {
    root->children[i] = (position_node *)malloc(sizeof(position_node));
  }

  char free_squares = s->free_squares;
  for (char i = 0; i < root->num_children; i++) {
    s->free_squares = free_squares - i;
    s->pawns[side] = s->fixed_pawns[side] + i;

    if (side == 1) {
      count_from_pawns(s, root->children[i], !side, 0);
    } else {
      count_from_fixed_rooks_and_kings(s, root->children[i], 0);
    }

    mpz_mul_ui(root->children[i]->num_positions,
               root->children[i]->num_positions,
               binomials[free_squares - unoccupiable_adjacent -
                         (NUM_SIDES * BOARD_SIDE_LENGTH)][i]);
    mpz_add(root->num_positions, root->num_positions,
            root->children[i]->num_positions);
  }
}

void count_from_enpassant(position_node *root) {
  mpz_init(root->num_positions);
  root->num_children = 4;
  root->children =
      (position_node **)malloc(root->num_children * sizeof(position_node *));
  for (char i = 0; i < root->num_children; i++) {
    root->children[i] = (position_node *)malloc(sizeof(position_node));
  }

  position_state s = {0};
  // no enpassant
  s.free_squares = NUM_SQUARES;
  // Other side is always to move. Colour is assigned after tree search
  count_from_pawns(&s, root->children[NO_ENPASSANT], 1, 0);
  mpz_add(root->num_positions, root->num_positions,
          root->children[NO_ENPASSANT]->num_positions);

  // enpassant, enpassant pawn not on edge file, and only one adjacent
  // opposition pawn
  s.fixed_pawns[0] = 1;
  s.fixed_pawns[1] = 1;
  s.free_squares = NUM_SQUARES - 4;
  // we place side 2's pawns first to handle the empty enpassant adjacent square
  count_from_pawns(&s, root->children[ENPASSANT_ONE_ADJACENT], 1, 1);
  mpz_mul_ui(root->children[ENPASSANT_ONE_ADJACENT]->num_positions,
             root->children[ENPASSANT_ONE_ADJACENT]->num_positions,
             ENPASSANT_ONE_ADJACENT_VARIATIONS);
  mpz_add(root->num_positions, root->num_positions,
          root->children[ENPASSANT_ONE_ADJACENT]->num_positions);

  // enpassant pawn on edge file
  count_from_pawns(&s, root->children[ENPASSANT_EDGE], 1, 0);
  mpz_mul_ui(root->children[ENPASSANT_EDGE]->num_positions,
             root->children[ENPASSANT_EDGE]->num_positions,
             ENPASSANT_EDGE_VARIATIONS);
  mpz_add(root->num_positions, root->num_positions,
          root->children[ENPASSANT_EDGE]->num_positions);

  // enpassant with two adjancent opposition pawns
  s.fixed_pawns[1] = 2;
  s.free_squares = NUM_SQUARES - 5;
  count_from_pawns(&s, root->children[ENPASSANT_TWO_ADJACENT], 1, 0);
  mpz_mul_ui(root->children[ENPASSANT_TWO_ADJACENT]->num_positions,
             root->children[ENPASSANT_TWO_ADJACENT]->num_positions,
             ENPASSANT_TWO_ADJACENT_VARIATIONS);
  mpz_add(root->num_positions, root->num_positions,
          root->children[ENPASSANT_TWO_ADJACENT]->num_positions);
}

void build_sample_space(position_node *root) { count_from_enpassant(root); }
