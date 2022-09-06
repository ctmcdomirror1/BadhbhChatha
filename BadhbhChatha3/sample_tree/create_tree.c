#include <assert.h>
#include <gmp.h>
#include <stdbool.h>
#include <stdlib.h>

#include "chess.h"
#include "tree_common.h"
#include "util.h"

// I document this file from the end upwards, so you'd be best starting
// from the bottom. The comments get less useful towards the top of the
// file because I'm trying to finish up this project. There is most likely
// at least one bug in this file as I expect the tree to represent around
// E55 positions but it only counts 2E44 positions

#define min3(a, b, c) min(min(a, b), c)

char factorials[NUM_PIECE_TYPES_LESS_KING] = {1, 2, 6, FOUR_FACTORIAL};
char pieces[NUM_SIDES][NUM_PIECE_TYPES_LESS_KING] = {0};

typedef struct position_state {
  char occupiable_squares;
  char fixed_pawns[NUM_SIDES];
  bool fixed_king[NUM_SIDES];
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

// This could be a lot more efficient but we don't need it to be. Speeding
// up the permutation calculation would have a huge impact on the time to
// build the tree, and there surely are ways to do so
short num_piece_type_permutations(
    char base_piece_limits_pside[NUM_SIDES][NUM_PIECE_TYPES_LESS_KING],
    char pawns[NUM_SIDES],
    char num_pieces[NUM_SIDES][NUM_PIECE_TYPES_LESS_KING],
    char total_pieces[NUM_SIDES]) {

  // Certain permutations cost too much hence we use limits. This is
  // certainly something that could be optimised, there's no inference
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
  // assert(legal_permutations > 0);

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
  // assert((legal_permutations % factors_product) == 0);
  legal_permutations /= factors_product;

  return legal_permutations;
}

void count_from_pieces_helper(
    const char occupiable_squares, const bool fixed_king[NUM_SIDES],
    char pawns[NUM_SIDES],
    char base_piece_limits_pside[NUM_SIDES][NUM_PIECE_TYPES_LESS_KING],
    char num_pieces[NUM_SIDES][NUM_PIECE_TYPES_LESS_KING],
    two_entry_wrapped_char_array total_pieces,
    two_entry_wrapped_char_array promotions, position_node *root,
    const char rel_level, two_entry_wrapped_char_array previous_pieces) {
  mpz_init(root->num_positions);

  // rel_level is used to determine piece type and side
  const bool side = rel_level >= NUM_PIECE_TYPES_LESS_KING;
  const char piece_type = (int)(rel_level % NUM_PIECE_TYPES_LESS_KING);
  char new_base_pieces_limit = base_piece_limits_pside[side][piece_type];

  // There are a number of limits on the number of pieces.
  // 1. We can only have at most the base pieces plus 2 times the number of missing
  // opposition pawns plus missing opposition pieces less pieces which must be
  // promoted on both sides. The reasoning is for a pawn to promote it must move
  // past an opposition pawn or its path be cleared. For every missing opposition
  // pawn we assume it was captured by a pawn such that it can promote and a pawn
  // on capture file can also promote. Every missing opposition piece is similarly
  // a chance for a pawn to get behind an opposition pawn. We subtract known promotions
  char new_pieces_lim1 = new_base_pieces_limit +
                         2 * (NUM_PAWNS_PSIDE - pawns[!side]) +
                         (NUM_PIECES_PSIDE_LESS_KING - total_pieces.A[!side]) -
                         promotions.A[!side] - promotions.A[side];
  // 2. Similar to case 1, I can't remember how I worked out this tbh. I worked it
  // out at one point that there was symmetry between the constraints for both sides
  char new_pieces_lim2 = 2 * (NUM_PAWNS_PSIDE - pawns[side]) +
                         (NUM_PIECES_PSIDE_LESS_KING - total_pieces.A[side]) -
                         promotions.A[!side] - promotions.A[side];
  // ... once it was lte the base piece limit
  if (new_pieces_lim2 > new_base_pieces_limit) {
    // and I'm also not sure what I was doing here. I have feeling I put a good
    // deal of thought into this but it could be wrong
    new_pieces_lim2 = (char)((new_pieces_lim2 - new_base_pieces_limit) / 2);
  }
  // 3. We can only have as many promotions as pawns we're missing
  char new_pieces_lim3 = new_base_pieces_limit + NUM_PAWNS_PSIDE - pawns[side] -
                         promotions.A[side];
  char new_pieces_lim = min3(new_pieces_lim1, new_pieces_lim2, new_pieces_lim3);
  // assert(0 <= new_pieces_lim && new_pieces_lim <= 10);

  // Always have the 0 of this piece type case. We limit the number of pieces
  // as the absolute pieces limit just computed and the previous number of
  // pieces. The number of pieces for the same side is therefore decreasing,
  // and we later consider how many ways can we map actual piece types to these
  // generic pieces. This is the reason why we wanted base_piece_types to be decreasing
  root->num_children = min(1 + new_pieces_lim, 1 + previous_pieces.A[side]);
  // assert(0 <= root->num_children && root->num_children <= 11);
  root->children =
      (position_node **)malloc(root->num_children * sizeof(position_node *));
  for (char i = 0; i < root->num_children; i++) {
    root->children[i] = (position_node *)malloc(sizeof(position_node));
  }

  for (int i = 0; i < root->num_children; i++) {
    // assert(0 <= occupiable_squares - i && occupiable_squares - i <= 64);
    // assert(total_pieces.A[side] + i <= 15);
    char new_promotions_of_this = 0;
    if (i > base_piece_limits_pside[side][piece_type]) {
      new_promotions_of_this = i - base_piece_limits_pside[side][piece_type];
    }
    // assert(0 <= new_promotions_of_this && new_promotions_of_this <= 8);

    num_pieces[side][piece_type] = i;
    if ((side == 1 && i == 0) ||
        rel_level == (2 * NUM_PIECE_TYPES_LESS_KING - 1)) {

      uint64_t variations = binomials[occupiable_squares][i];
      two_entry_wrapped_char_array new_total_pieces;
      new_total_pieces.A[!side] = total_pieces.A[!side];
      new_total_pieces.A[side] = total_pieces.A[side] + i;
      variations *= num_piece_type_permutations(base_piece_limits_pside, pawns,
                                                num_pieces, new_total_pieces.A);

      char kfree_squares = free_squares - i;
      // if no fixed king then we place the king
      if (!fixed_king[0]) {
        variations *= kfree_squares;
        kfree_squares--;
      }
      if (!fixed_king[1]) {
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
          occupiable_squares - i, fixed_king, pawns, base_piece_limits_pside,
          num_pieces, new_total_pieces, new_promotions, root->children[i],
          new_rel_level, new_previous_pieces);

      mpz_mul_ui(root->children[i]->num_positions,
                 root->children[i]->num_positions, binomials[occupiable_squares][i]);
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
  // MAX_BISHOPS_PSIDE = 10 is greater than or equal to max of any other piece type
  previous_pieces.A[0] = MAX_BISHOPS_PSIDE;
  previous_pieces.A[1] = MAX_BISHOPS_PSIDE;

  two_entry_wrapped_char_array promotions = {0};
  assert(0 == promotions.A[0]);
  count_from_pieces_helper(s->occupiable_squares, s->fixed_king, s->pawns,
                           s->base_piece_limits_pside, num_pieces, total_pieces,
                           promotions, root, 0, previous_pieces);
}

// Level 3 is for placing rooks with castling rights and hence also the fixed
// king on its home square.
void count_from_fixed_rooks_and_kings(position_state *s, position_node *root,
                                      const bool side) {
  root->num_children = 3;
  root->children =
      (position_node **)malloc(root->num_children * sizeof(position_node *));
  for (char i = 0; i < root->num_children; i++) {
    root->children[i] = (position_node *)malloc(sizeof(position_node));
  }

  typedef void (*fn)(position_state *, position_node *, bool side);
  fn next_call = count_from_fixed_rooks_and_kings;
  if (side) {
    next_call = count_from_pieces;
  }

  // Base piece limits represent the maximum number of each piece type
  // (less the king) which we can place and which could be considered base
  // pieces (non-promoted). Rooks with castling rights are not considered
  // and are the only reason this array isn't a constant.
  // Here we just copy over the limits of b<-2, n<-2, r<-2, q<-1
  for (char i = 0; i < NUM_PIECE_TYPES_LESS_KING; i++) { // = |{b, n, r, q}| = 4
    // BASE_PIECE_LIMITS being initialised from main/util
    s->base_piece_limits_pside[side][i] = BASE_PIECE_LIMITS[i];
  }

  char occupiable_squares = s->occupiable_squares;
  // CASE 1: no castling rights
  s->fixed_king[side] = false;
  // We don't consider king in s->total_pieces
  s->total_pieces[side] = 0;
  next_call(s, root->children[NO_CASTLING_RIGHTS], !side);

  // CASE 2: castling rights on one side
  // We shouldn't need to reset base_piece_limits as it shouldn't be altered
  // outside of this function
  s->fixed_king[side] = true;
  // The king is on its home square and one rook is on its home square
  s->occupiable_squares = occupiable_squares - 2;
  // We then have one less rook we can place and consider non-promoted
  s->base_piece_limits_pside[side][ROOK] = 1;
  s->total_pieces[side] = 1;
  next_call(s, root->children[CASTLING_RIGHTS_ONE_SIDE], !side);
  mpz_mul_ui(root->children[CASTLING_RIGHTS_ONE_SIDE]->num_positions,
             root->children[CASTLING_RIGHTS_ONE_SIDE]->num_positions,
             ONE_FIXED_ROOK_VARIATIONS); // 2, either side

  // CASE 3: castling rights on both sides
  s->occupiable_squares = occupiable_squares - 3;
  // Our base piece limits should be b<-2, n<-2, r<-0, q<-1, but for reasons
  // that make more sense when considering count_from_pieces, we'd like
  // base piece limits to be decreasing, so we swap the rook and queen slots
  // so we now have 2, 2, 1, 0
  s->base_piece_limits_pside[side][QUEEN] = 0;
  s->total_pieces[side] = 2;
  next_call(s, root->children[CASTLING_RIGHTS_BOTH_SIDES], !side);
  
  mpz_init(root->num_positions);
  for (char i = 0; i < 3; i++) {
  mpz_add(root->num_positions, root->num_positions,
          root->children[i]->num_positions);
  }
}

// The 2nd level represents free pawns (pawns not associated with enpassant).
// We count from side1 first to simplify enpassant accounting
void count_from_pawns(position_state *s, position_node *root, const bool side,
                      const bool unoccupiable_adjacent) {
  // 'root' is the subtree root of course.
  // We can place up to 8 pawns less pawns associated with enpassant as the
  // case may be
  root->num_children = 1 + (NUM_PAWNS_PSIDE - s->fixed_pawns[side]);
  root->children =
      (position_node **)malloc(root->num_children * sizeof(position_node *));
  for (char i = 0; i < root->num_children; i++) {
    root->children[i] = (position_node *)malloc(sizeof(position_node));
  }

  // Save s->occupiable_squares entry value
  char occupiable_squares = s->occupiable_squares;
  for (char i = 0; i < root->num_children; i++) {
    s->occupiable_squares = occupiable_squares - i;
    // s->fixed_pawns shouldn't be altered by future calls so we shouldn't
    // need to save fixed_pawns' entry value
    s->pawns[side] = s->fixed_pawns[side] + i;

    if (side == 1) {
      count_from_pawns(s, root->children[i], 0, false);
    } else {
      // We resume counting for side0 before side1. Accounting for side1 before
      // side0 in this function is the exception
      count_from_fixed_rooks_and_kings(s, root->children[i], 0);
    }
    mpz_mul_ui(root->children[i]->num_positions,
               root->children[i]->num_positions,
               binomials[occupiable_squares - unoccupiable_adjacent -
                         (NUM_SIDES * BOARD_SIDE_LENGTH)][i]);
    // The unoccupiable_adjacent should only be 1 in case of non-edge enpassant
    // with 1 adjacent pawn. We subtract 16 squares from available squares because
    // pawns can't be placed on the 1st and 8th ranks
  }
  
  mpz_init(root->num_positions);
  for (char i = 0; i < root->num_children; i++) {
    mpz_add(root->num_positions, root->num_positions,
            root->children[i]->num_positions);
  }
}

// The first level represents the enpassant characteristic.
// Instead of representing enpassant cases for white and black with two
// different subtrees we abstract the notion of black and white with side0 and
// side1. We enforce that only side0 can produce an enpassant square by using
// the stricter condition that side1 is the side to move next. We can then
// represent positions by assigning side0 one of {white, black}.
// An idea: thread the calculation of different enpassant subtrees or even
// within count_from_pawns to speed up the tree build
void count_from_enpassant(position_node *root) {
  root->num_children = 4;
  root->children =
      (position_node **)malloc(root->num_children * sizeof(position_node *));
  for (char i = 0; i < root->num_children; i++) {
    root->children[i] = (position_node *)malloc(sizeof(position_node));
  }

  // We keep state for passing information onto further levels such as the
  // number of free squares, the number of pawns, the number of pieces etc.
  // The struct is defined at the top of this file.
  position_state s = {0};
  // CASE 1: there's no enpassant square
  // in which case there are currently 64 unoccupied squares
  s.occupiable_squares = NUM_SQUARES; // == 64
  // We handle side1 count_from_pawns before side0 count_from_pawns because of a
  // detail in CASE 2.
  // We pass in the corresponding child node so that its subtree size can be stored
  count_from_pawns(&s, root->children[NO_ENPASSANT], 1, false);
  // We only consider there to be 1 way to have no enpassant square

  // CASE 2: there is an enpassant square and it's not on an edge file (a and h),
  // meaning that it's on a file within [b, g] and there's just the one adjacent
  // opposition pawn (one adjacent side1 pawn). Why consider edge enpassant and
  // not just enpassant in general? Because depending on the enpassant file, and
  // whether there are 1 or 2 adjacent side1 pawns the number of boards with these
  // specific enpassant characteristics are different, and hence we can't generate
  // them uniformly
  s.fixed_pawns[0] = 1;
  s.fixed_pawns[1] = 1;
  // Two squares are occupied but there also can't be anything on the 2 squares
  // behind the side0 enpassant pawn, so 4 squares are occupied/currently-unoccupiable
  s.occupiable_squares = NUM_SQUARES - 4;
  // We place side1's pawns first because we don't want to allow for placing
  // side0 pawns on one of the adjacent squares just yet. It's just simpler to
  // account for the side1 adjacent pawn being placed on either of the adjacent squares
  // after which we can then place the side0 pawns without restriction.
  // The 2nd argument to count_from_pawns represents whether one of the adjacent squares
  // is unoccupiable, and only applies to side1 pawns.
  count_from_pawns(&s, root->children[ENPASSANT_ONE_ADJACENT], 1, true);
  mpz_mul_ui(root->children[ENPASSANT_ONE_ADJACENT]->num_positions,
             root->children[ENPASSANT_ONE_ADJACENT]->num_positions,
             ENPASSANT_ONE_ADJACENT_VARIATIONS); // 6 * 2
  // a side0 pawn on one of the |[b, g]| = 6 files is unique and the side1 pawn
  // can be plaecd on either side of the side0 pawn

  // CASE 3: there is an enpassant square and it's on an edge file. There can
  // obviously only be just the one adjacent side1 pawn because we're on an edge file.
  // We have to reset the free squares because it's altered in previous calls. Why not
  // just pass it as a parameter? Well it's just one less parameter, no good reason tbf
  s.occupiable_squares = NUM_SQUARES - 4;
  count_from_pawns(&s, root->children[ENPASSANT_EDGE], 1, false);
  mpz_mul_ui(root->children[ENPASSANT_EDGE]->num_positions,
             root->children[ENPASSANT_EDGE]->num_positions,
             ENPASSANT_EDGE_VARIATIONS); // 2
  // There are two edge files and the adjacent side1 pawn is forced to be on the inside
  // regardless

  // CASE 4: there is an enpassant square, it's on a non-edge file and there are two
  // adjancent side1 pawns
  // s.fixed_pawns[0] = 1;
  s.fixed_pawns[1] = 2;
  s.occupiable_squares = NUM_SQUARES - 5;
  count_from_pawns(&s, root->children[ENPASSANT_TWO_ADJACENT], 1, false);
  mpz_mul_ui(root->children[ENPASSANT_TWO_ADJACENT]->num_positions,
             root->children[ENPASSANT_TWO_ADJACENT]->num_positions,
             ENPASSANT_TWO_ADJACENT_VARIATIONS); // 6
  // 6 files for non-edge file and only 1 way to place both adjacent pawns ofc 
  
  mpz_init(root->num_positions);
  // Set the total number of positions as the sum of child subtree sizes
  for (char i = 0; i < 4; i++) {
    mpz_add(root->num_positions, root->num_positions,
            root->children[i]->num_positions);
  }
}

// Our sample space is a tree. Each level of the tree represents a
// characteristic of a board, where boards with the exact same characteristics
// are represented by the same leaf. The tree doesn't store every position of course,
// that would be far too much memory as the number of positions is likely between
// 10e44 and 10e45. We abstract where exactly most pieces are. We include as much
// structure, i.e. where chessmen are or are bound within as we think is useful.
// As potentially reachable positions which we generate from searching the tree we
// build in this file will be filtered by considering unreachable conditions, we
// can keep certain types of unreachable positions in our tree for later filtering
// where it makes sense. Ultimately the tree only has to be optimised to a point
// where it can be stored in a reasonable amount of memory and built within a
// reasonable amount of time.
// The 1st level of the tree (root being the 0th) represents enpassant, and the
// first scenario is no enpassant square which is the first child of root. We use
// a depth-first recursive approach so every board with no enpassant square is
// considered before boards with enpassant pawns. And every board with no enpassant
// square and 1 side0 pawn is considered before boards with no enpassaant pawn and
// 2 side0 pawns etc. We choose the level meanings s.t. we represent any board at most
// once, reduce the number of unreachable boards in the tree, and minimise the tree
// size where possible. For instance, the general pieces level doesn't consider where
// exactly pieces are placed and we minimise unreachable positions by considering the
// maximum number of pieces each side can have depending on how many pieces must be
// promoted. Nodes in the tree are of type position_node which contains a GMP
// mpz_t for the size of the subtree rooted at the node, a char for the number
// of child nodes and an array of child nodes.
void build_sample_space(position_node *root) { count_from_enpassant(root); }
