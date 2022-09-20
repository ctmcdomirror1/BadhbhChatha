#include <assert.h>
#include <gmp.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>

#include "chess.h"
#include "tree_common.h"
#include "util.h"

// I document this file from the end upwards, so you'd be best starting
// from the bottom.

#define min3(a, b, c) min(min(a, b), c)

char factorials[NUM_PIECE_TYPES_LESS_KING] = {1, 2, 6, FOUR_FACTORIAL};
char pieces[NUM_SIDES][NUM_PIECE_TYPES_LESS_KING] = {0};

// We use precomputed permutations here but not in the search tree in an
// effort to spot bugs in the search tree implementation.
char permutationsOf0to3[][NUM_PIECE_TYPES_LESS_KING] = {
    {0, 1, 2, 3}, {0, 1, 3, 2}, {0, 3, 1, 2}, {3, 0, 1, 2}, {0, 2, 1, 3},
    {0, 2, 3, 1}, {0, 3, 2, 1}, {3, 0, 2, 1}, {2, 0, 1, 3}, {2, 0, 3, 1},
    {2, 3, 0, 1}, {3, 2, 0, 1}, {1, 0, 2, 3}, {1, 0, 3, 2}, {1, 3, 0, 2},
    {3, 1, 0, 2}, {1, 2, 0, 3}, {1, 2, 3, 0}, {1, 3, 2, 0}, {3, 1, 2, 0},
    {2, 1, 0, 3}, {2, 1, 3, 0}, {2, 3, 1, 0}, {3, 2, 1, 0}};

typedef struct {
  char A[NUM_SIDES];
} two_entry_wrapped_char_array;

typedef struct {
  char fixed_pawns[NUM_SIDES];
  char fixed_rooks[NUM_SIDES];
  char pawns[NUM_SIDES];
  char occupiable_squares_before_free_pieces;
} position_state;

typedef struct {
  position_node *root;
  position_state *s;
  bool side;
} threading_struct;

void map_permutation(char input[NUM_PIECE_TYPES_LESS_KING],
                     char mapping[NUM_PIECE_TYPES_LESS_KING],
                     char permutation[NUM_PIECE_TYPES_LESS_KING]) {
  for (int i = 0; i < NUM_PIECE_TYPES_LESS_KING; i++) {
    permutation[i] = input[mapping[i]];
  }
}

// TODO: Speeding up the permutation calculation would have a huge impact on the
// time to build the tree, and there surely are ways to do so since we currently
// make no inferences, just brute force
short num_piece_type_permutations(
    char base_pieces_less_fr[NUM_SIDES][NUM_PIECE_TYPES_LESS_KING],
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
    char np0 = num_promotions(permutation, base_pieces_less_fr[0]);
    if (np0 > prom_lim1) {
      continue;
    }

    for (int j = 0; j < FOUR_FACTORIAL; j++) {
      map_permutation(num_pieces[1], permutationsOf0to3[j], permutation);
      char np1 = num_promotions(permutation, base_pieces_less_fr[1]);
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

// Level 6 onwards represents generic pieces. We don't assign a piece type until
// later, So we can tell we have x number of this piece type, y number of this
// piece type etc. We require two 0 levels or max level to reach leaf.
void count_from_pieces_helper(
    position_node *root, position_state *s,
    char num_pieces[NUM_SIDES][NUM_PIECE_TYPES_LESS_KING],
    char total_pieces[NUM_SIDES], char promotions[NUM_SIDES],
    const char rel_level) {
  // OBS: I think there would be a lot of identical subtrees depending on the
  // number of occupiable squares after pawns are placed (right before this
  // function). Even if only considering side1, depending on the allowable
  // promotions should be able to radically cut down on number of subtrees. In
  // which case the structure would be a DAG not a tree
  mpz_init(root->num_positions);

  // rel_level is used to determine piece type and side
  const bool side = rel_level >= NUM_PIECE_TYPES_LESS_KING;
  const char piece_type = (int)(rel_level % NUM_PIECE_TYPES_LESS_KING);
  char new_base_pieces_limit = base_pieces_less_fr[side][piece_type];

  // I had the following written up elsewhere:
  // Naturally, the number of promotions a side has must be less than or equal
  // to the number of pawns that that side is missing: $$prom \leq 8 - pawns$$
  // A side's promotions is also constrained by its opposition's composition.
  // Opposition pawns on a particular file start on opposing sides of the board
  // \textit{s.t.~}if they were to move forward without capturing they would
  // eventually deadlock. Therefore to promote, one pawn must either (1) (TODO:
  // inline list) be captured - unblocking the opposing pawn - or (2) make a
  // capture on a square which leaves the capturing pawn on its
  // promotion-rank-side of any opposition pawns on the capture file. We don't
  // partition positions by pawns on certain files (other than en-passant pawns)
  // so for every missing pawn we allow 2 opposition promotions, that is we
  // assume a pawn made the capture which also freed the opposition pawn which
  // started on the capture file which assumedly hasn't left the file:
  // $$prom1 \leq 2 \times ((8 - pawns2) - prom2) + (7 - (pieces2 - prom2))$$
  // $$\Rightarrow prom1 + prom2 \leq 2 \times (8 - pawns2) + (7 - pieces2)$$
  char new_pieces_lim1 = new_base_pieces_limit +
                         2 * (NUM_PAWNS_PSIDE - pawns[!side]) +
                         (NUM_PIECES_PSIDE_LESS_KING - total_pieces.A[!side]) -
                         promotions.A[!side] - promotions.A[side];
  // 2. TODO: Similar to case 1, I can't remember how I worked out this tbh. I
  // worked it out at one point that there was symmetry between the constraints
  // for both sides
  char new_pieces_lim2 = 2 * (NUM_PAWNS_PSIDE - pawns[side]) +
                         (NUM_PIECES_PSIDE_LESS_KING - total_pieces.A[side]) -
                         promotions.A[!side] - promotions.A[side];
  // ... once it was lte the base piece limit
  if (new_pieces_lim2 > new_base_pieces_limit) {
    // TODO: and I'm also not sure what I was doing here. I have feeling I put a
    // good deal of thought into this but it could be wrong
    new_pieces_lim2 = (char)((new_pieces_lim2 - new_base_pieces_limit) / 2);
  }
  // 3. We can only have as many promotions as pawns we're missing
  char new_pieces_lim3 = new_base_pieces_limit + NUM_PAWNS_PSIDE - pawns[side] -
                         promotions.A[side];
  char new_pieces_lim = min3(new_pieces_lim1, new_pieces_lim2, new_pieces_lim3);
  assert(0 <= new_pieces_lim && new_pieces_lim <= 10);

  // Always have the 0 of this piece type case. We limit the number of pieces
  // as the absolute pieces limit just computed and the previous number of
  // pieces. The number of pieces for the same side is therefore decreasing,
  // and we later consider how many ways can we map actual piece types to these
  // generic pieces. This is the reason why we wanted base_piece_types to be
  // decreasing
  root->num_children = min(1 + new_pieces_lim, 1 + previous_pieces.A[side]);
  assert(0 <= root->num_children && root->num_children <= 11);
  root->children =
      (position_node **)malloc(root->num_children * sizeof(position_node *));
  for (char i = 0; i < root->num_children; i++) {
    root->children[i] = (position_node *)malloc(sizeof(position_node));
  }

  for (int i = 0; i < root->num_children; i++) {
    assert(0 <= occupiable_squares - i && occupiable_squares - i <= 64);
    assert(total_pieces.A[side] + i <= 15);
    char new_promotions_of_this = 0;
    if (i > base_pieces_less_fr[side][piece_type]) {
      new_promotions_of_this = i - base_pieces_less_fr[side][piece_type];
    }
    assert(0 <= new_promotions_of_this && new_promotions_of_this <= 8);

    num_pieces[side][piece_type] = i;
    // if we're at a leaf
    if ((side == 1 && i == 0) ||
        rel_level == (2 * NUM_PIECE_TYPES_LESS_KING - 1)) {

      uint64_t variations = binomials[occupiable_squares][i];
      two_entry_wrapped_char_array new_total_pieces;
      new_total_pieces.A[!side] = total_pieces.A[!side];
      new_total_pieces.A[side] = total_pieces.A[side] + i;
      variations *= num_piece_type_permutations(base_pieces_less_fr, pawns,
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

      // else we're not at a leaf
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
          occupiable_squares - i, pawns, base_pieces_less_fr, num_pieces,
          new_total_pieces, new_promotions, root->children[i], new_rel_level,
          new_previous_pieces);

      mpz_mul_ui(root->children[i]->num_positions,
                 root->children[i]->num_positions,
                 binomials[occupiable_squares][i]);
      mpz_add(root->num_positions, root->num_positions,
              root->children[i]->num_positions);
    }
  }

  num_pieces[side][piece_type] = 0;
}

// The 4th and 5th levels represent free (not fixed) pawns
void count_from_pawns(position_node *root, position_state *pstate,
                      const bool side, const char occupiable_squares,
                      const bool unoccupiable_adjacent) {
  root->num_children = 1 + (NUM_PAWNS_PSIDE - pstate->fixed_pawns[side]);
  root->children =
      (position_node **)malloc(root->num_children * sizeof(position_node *));
  for (char i = 0; i < root->num_children; i++) {
    root->children[i] = (position_node *)malloc(sizeof(position_node));
  }

  for (char i = 0; i < root->num_children; i++) {
    if (side) {
      count_from_pawns(root->children[i], pstate, 0, unoccupiable_squares + i,
                       false);
    } else {
      // MAX_BISHOPS_PSIDE = 10 is greater than or equal to max of any other
      // piece type. This is important because count_from_pieces_helper counts
      // up to whatever the previous number of pieces were. By enforcing this
      // order we know that we're not duplicating combinations of pieces, where
      // we've yet to actually assign pieces types previous_pieces.A[0] =
      // MAX_BISHOPS_PSIDE; previous_pieces.A[1] = MAX_BISHOPS_PSIDE;

      char free_pieces[NUM_SIDES][NUM_PIECE_TYPES_LESS_KING] = {0};
      char total_free_pieces[NUM_SIDES] = {0};
      char promotions[NUM_SIDES] = {0};
      count_from_pieces(root->children[i], pstate, num_pieces,
                        total_free_pieces, promotions, 0);
    }
    mpz_mul_ui(root->children[i]->num_positions,
               root->children[i]->num_positions,
               binomials[occupiable_squares - unoccupiable_adjacent -
                         (NUM_SIDES * BOARD_SIDE_LENGTH)][i]);
    // The unoccupiable_adjacent should only be 1 in case of non-edge enpassant
    // with 1 adjacent pawn. We subtract 16 squares from available squares
    // because pawns can't be placed on the 1st and 8th ranks
  }

  mpz_init(root->num_positions);
  for (char i = 0; i < root->num_children; i++) {
    mpz_add(root->num_positions, root->num_positions,
            root->children[i]->num_positions);
  }
}

// Level 2 and 3 account for rooks with castling rights
// (fixed rooks). Level 1 places side0's fixed rooks, and level 2 places side1's
// fixed rooks. The reason for abstracting black and white to side0 and side1 is
// related to enpassant; I leave a comment explaining the reasoning above the
// count_from_enpassant function. In cases/scenarios with fixed rooks we also
// account for the king on its home square. We thread from this function, and
// since there are 3 cases per side there are 3 * 3 = 9 threads used to build
// the sample structure
void *count_from_fixed_rooks(void *arg) {
  threading_struct ts = *(threading_struct *)arg;
  ts.root->num_children = 3;
  ts.root->children =
      (position_node **)malloc(root->num_children * sizeof(position_node *));
  for (char i = 0; i < root->num_children; i++) {
    ts.root->children[i] = (position_node *)malloc(sizeof(position_node));
  }

  void *next_call = count_from_fixed_rooks;
  if (ts.side) {
    next_call = count_from_enpassant;
  }

  // CASE 1: no castling rights
  // We keep state for passing information onto further levels such as the
  // number of free squares, the number of pawns, the number of pieces etc.
  // The struct is defined at the top of this file.
  position_state s1 = {0};
  // We don't consider king in total_pieces so no need to set s1.total_pieces.
  // Nor de we even place the king for CASE 1 since it's not fixed.
  s1.occupiable_squares = NUM_SQUARES;
  pthread_t thread_ids[3];
  threading_struct ts1;
  ts1.s = &s1;
  ts1.root = ts.root->children[NO_CASTLING_RIGHTS];
  ts1.side = !ts.side;
  pthread_create(&thread_ids[0], NULL, next_call, &ts1);

  // CASE 2: castling rights on one side
  position_state s2 = {0};
  s2.fixed_rooks[ts.side] = 1;
  // The king is on its home square and one rook is on its home square
  s2.occupiable_squares = NUM_SQUARES - 2;
  s2.total_pieces[ts.side] = 1;
  threading_struct ts2;
  ts2.s = &s2;
  ts2.root = ts.root->children[CASTLING_RIGHTS_ONE_SIDE];
  ts2.side = !ts.side;
  pthread_create(&thread_ids[1], NULL, next_call, &ts2);

  // CASE 3: castling rights on both sides
  position_state s3 = {0};
  s3.fixed_rooks[ts.side] = 2;
  s3.occupiable_squares = NUM_SQUARES - 3;
  s3.total_pieces[ts.side] = 2;
  threading_struct ts3;
  ts3.s = &s3;
  ts3.root = ts.root->children[CASTLING_RIGHTS_ONE_SIDE];
  ts3.side = !ts.side;
  pthread_create(&thread_ids[2], NULL, next_call, &ts3);

  for (char i = 0; i < 3; i++) {
    pthread_join(thread_ids[i], NULL);
  }

  mpz_mul_ui(ts.root->children[CASTLING_RIGHTS_ONE_SIDE]->num_positions,
             ts.root->children[CASTLING_RIGHTS_ONE_SIDE]->num_positions,
             ONE_FIXED_ROOK_VARIATIONS); // 2; fixed rook can be either side

  mpz_init(ts.root->num_positions);
  for (char i = 0; i < 3; i++) {
    mpz_add(ts.root->num_positions, root->num_positions,
            ts.root->children[i]->num_positions);
  }
}

// Level 1 (0 being the root) represents enpassant scenarios. Instead of
// representing enpassant cases for white and black with two different subtrees
// we abstract the notion of black and white with side0 and side1. We enforce
// that only side0 can produce an enpassant square by using the stricter
// condition that side1 is the side to move next. We can then represent
// positions by assigning side0 one of {white, black}
void *count_from_enpassant(void *arg) {
  threading_struct ts = *(threading_struct *)arg;
  ts.root->num_children = 4;
  ts.root->children =
      (position_node **)malloc(ts.root->num_children * sizeof(position_node *));
  for (char i = 0; i < ts.root->num_children; i++) {
    ts.root->children[i] = (position_node *)malloc(sizeof(position_node));
  }

  position_state s = {0};
  for (char side = 0; side < NUM_SIDES; side++) {
    s.fixed_rooks[side] = ts->s->fixed_rooks[side];
    s.total_pieces[side] = ts->s->total_pieces[side];
  }

  // OBS: We should treat no enpassant and enpassant with 1 adjacent pawn
  // in the same manner: cut out up to half further possbibilities due to
  // symmetry. This goes for fixed rooks, and pawns, pieces etc.
  // CASE 1: there's no enpassant square
  char o = ts->s->fixed_rooks[1] > ts->s->fixed_rooks[0];
  // So we look to (at most) halve our work
  if (o) {
    // We determine the number of pawns in side1 (via a call to
    // count_from_pawns) before side0 even though we normally count for side0
    // before side1. We do this because if the enpassant square is a non-edge
    // file ([b, g]) we have 2 scenarios: there are either 1 or 2 enpassant-pawn
    // rank-adjacent squares occupied by side1 pawns. We need to consider both
    // scenarios separately to avoid counting duplicate positions. We handle
    // this temporarily unoccupiable adjacency ASAP by accounting for side1
    // before side0 pawns after which the potentially empty adjacency would then
    // be considered once again occupiable. The last/rightmost parameter in
    // count_from_pawns is 'unoccupiable_adjacent'
    count_from_pawns(ts.root->children[NO_ENPASSANT], &s, 1,
                     ts->s->occupiable_squares, false);
  }

  // TODO: some other equivlance?
  // CASE 2: there's an enpassant square on a non-edge file with 1 side1
  // occupied enpassant-pawn rank-adjacent square
  s.fixed_pawns[0] = 1;
  s.fixed_pawns[1] = 1;
  // Two squares are occupied but there also can't be anything on the enpassant
  // square or enpassant-pawn's starting square
  count_from_pawns(ts.root->children[ENPASSANT_ONE_ADJACENT], &s, 1,
                   ts->s->occupiable_squares - 4, true);
  mpz_mul_ui(ts.root->children[ENPASSANT_ONE_ADJACENT]->num_positions,
             ts.root->children[ENPASSANT_ONE_ADJACENT]->num_positions,
             ENPASSANT_ONE_ADJACENT_VARIATIONS); // 6 * 2
  // A side0 pawn on one of the |[b, g]| = 6 files is unique and the side1 pawn
  // can be placed on either side of the enpassant-pawn

  // TODO: symmetry?
  // CASE 3: there's an enpassant square on an edge file
  count_from_pawns(ts.root->children[ENPASSANT_EDGE], &s, 1,
                   ts->s->occupiable_squares - 4, false);
  mpz_mul_ui(ts.root->children[ENPASSANT_EDGE]->num_positions,
             ts.root->children[ENPASSANT_EDGE]->num_positions,
             ENPASSANT_EDGE_VARIATIONS); // 2

  // TODO: symmetry?
  // CASE 4: there is an enpassant square, it's on a non-edge file and there are
  // two enpassant-pawn rank-adjancent side1 pawns
  s.fixed_pawns[1] = 2;
  count_from_pawns(ts.root->children[ENPASSANT_TWO_ADJACENT], &s, 1,
                   ts->s->occupiable_squares - 5, false);
  mpz_mul_ui(ts.root->children[ENPASSANT_TWO_ADJACENT]->num_positions,
             ts.root->children[ENPASSANT_TWO_ADJACENT]->num_positions,
             ENPASSANT_TWO_ADJACENT_VARIATIONS); // 6 * 1

  mpz_init(ts.root->num_positions);
  for (char i = 0 + o; i < 4; i++) {
    mpz_add(ts.root->num_positions, ts.root->num_positions,
            ts.root->children[i]->num_positions);
  }
}

// Our sample space is a tree. Each level of the tree represents a
// characteristic of a board, where boards with the exact same characteristics
// are represented by the same leaf. The tree doesn't store every position of
// course, that would be far too much memory as the number of positions is
// likely between 10e44 and 10e45. We abstract where exactly most pieces are. We
// include as much structure - where chessmen are or are bound within - as we
// would like to use. As potentially reachable positions which we generate from
// searching the tree we build in this file will be filtered by considering
// unreachable conditions, we can keep certain types of unreachable positions in
// our tree for later filtering where it makes sense. Ultimately the tree only
// has to be optimised to a point where it can be stored in a reasonable amount
// of memory and built within a reasonable amount of time. We build the tree in
// a DFS manner. Logically the sample structure is a tree, but in practice we
// try point nodes which have identical subtrees to the same subtree root s.t.
// we have a DAG. Nodes in the tree are of type position_node which contains a
// GMP mpz_t for the size of the subtree rooted at the node, a char for the
// number of child nodes and an array of child nodes
void build_sample_space(position_node *root) {
  threading_struct ts = {0};
  ts.root = root;
  ts.side = 0;
  count_from_fixed_rooks(&ts);
  mpz_mul_ui(root->num_positions, root->num_positions, 2);
}
