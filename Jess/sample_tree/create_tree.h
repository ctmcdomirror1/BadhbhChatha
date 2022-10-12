#include <assert.h>
#include <gmp.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>

#include "chess_constants.h"
#include "tree_common.h"
#include "util.h"

// I document this file from the end upwards, so you'd be best starting
// from the bottom.

// Could-Have-Done:
// Sort children by size of subtree to speed up sampling.

#define min3(a, b, c) min(min(a, b), c)
#define min4(a, b, c, d) min(a, min(min(b, c), d))

char factorials[NUM_PIECE_TYPES_LESS_KING] = {1, 2, 6, FOUR_FACTORIAL};
char pieces[NUM_SIDES][NUM_PIECE_TYPES_LESS_KING] = {0};

typedef struct {
  position_node *root;
  char pawns[NUM_SIDES];
  char fixed_rooks[NUM_SIDES];
  char occupiable_squares;
  bool enpassant;
  bool side;
} threading_struct;

char covered_set_index(char fr,
                       char capturable_pieces[NUM_PIECE_TYPES_LESS_KING]) {
  char cs[NUM_PIECE_TYPES_LESS_KING];
  for (char i = 0; i < NUM_PIECE_TYPES_LESS_KING; i++) {
    covered_set[i] = capturable_pieces[i];
    if (capturable_pieces[i] > BASE_PIECES[fr][i]) {
      covered_set[i] = BASE_PIECES[fr][i];
    }
  }
  return fr_coveredSet_indices[fixed_rooks_case][cs[0]][cs[1]][cs[2]][cs[3]];
}

char num_piece_type_permutations(
    char pawns[NUM_SIDES], char fr[NUM_SIDES],
    char capturable_pieces[NUM_SIDES][NUM_PIECE_TYPES_LESS_KING],
    char total_base_capturable_pieces[NUM_SIDES], char promotions[NUM_SIDES]) {

  char slack1_1 = NUM_PAWNS_PSIDE - pawns[side] - promotions[side];
  char slack1_2 =
      2 * (NUM_PAWNS_PSIDE - pawns[!side] - promotions[!side]) +
      (NUM_BASE_CAPTURABLE_PIECES_PSIDE - total_base_pieces[!side]) -
      promotions[side];
  char slack1_3 = 2 * (NUM_PAWNS_PSIDE - pawns[side] - promotions[side]) +
                  (NUM_BASE_CAPTURABLE_PIECES_PSIDE - total_base_pieces[side]) -
                  promotions[!side];
  char slack1 = min4(slack1_1, slack1_2, slack1_3, MAX_UNIQUE_COSTS);

  char variations = 1;
  char csi1 = covered_set_index(fr[0], capturable_pieces[0]);
  char csi2 = covered_set_index(fr[1], capturable_pieces[1]);
  char slack2_1 = NUM_PAWNS_PSIDE - pawns[!side] - promotions[!side];
  for (char i = 0; i < slack1; i++) {
    char slack2_2 =
        2 * (NUM_PAWNS_PSIDE - pawns[side] - (promotions[side] + slack)) +
        (NUM_BASE_CAPTURABLE_PIECES_PSIDE - (total_base_pieces[side] - slack)) -
        promotions[!side];
    char slack2_3 =
        2 * (NUM_PAWNS_PSIDE - pawns[!side] - (promotions[!side])) +
        (NUM_BASE_CAPTURABLE_PIECES_PSIDE - total_base_pieces[!side]) -
        (promotions[side] + slack);
    char slack2 = min4(slack2_1, slack2_2, slack2_3, MAX_UNIQUE_COSTS - 1);
    assert(slack2 >= 0);

    char p1 = fr_coveredSet_perm_cost_boundaries[fr[0]][i];
    if (i > 0) {
      p1 -= fr_coveredSet_perm_cost_boundaries[fr[0]][i - 1];
    }

    variations *= p1 * fr_coveredSet_perm_cost_boundaries[fr[1]][slack2];
  }

  return variations;
}

// Level 6 onwards represents generic pieces. We don't assign a piece type until
// later, So we can tell we have x number of this piece type, y number of this
// piece type etc. We require two 0 levels or max level to reach leaf.
void count_from_pieces_helper(
    position_node *root, char pawns[NUM_SIDES], char fixed_rooks[NUM_SIDES],
    char capturable_pieces[NUM_SIDES][NUM_PIECE_TYPES_LESS_KING],
    char total_base_capturable_pieces[NUM_SIDES], char promotions[NUM_SIDES],
    const char occupiable_squares, const char rel_level) {
  mpz_init(root->num_positions);

  // rel_level is used to determine side and piece_type
  const bool side = rel_level > NUM_PIECE_TYPES_LESS_KING;
  const char piece_type =
      (int)((rel_level - (side + 1)) % NUM_PIECE_TYPES_LESS_KING);

  char max_base_pieces = BASE_PIECES[fixed_rooks[side]][piece_type];
  // (1) Naturally, the number of promotions a side has must be less than or
  // equal to the number of pawns that that side is missing
  char pieces_lim1 =
      max_base_pieces + NUM_PAWNS_PSIDE - pawns[side] - promotions[side];

  // (2) Opposing pawns start on opposide sides of the board s.t. if they were
  // to move forward without capturing they would eventually deadlock. Therefore
  // to promote, one pawn must either (1) be captured - unblocking the file for
  // at least one opposing pawn or (2) make a capture leaving it on its
  // promotion-rank-side of any opposition pawns on the file. If a piece is
  // missing we can assume a pawn captured it. If a pawn is missing (and is not
  // considered a promotion) we can assume a pawn captured it and freed another
  // pawn on the capture file.
  char pieces_lim2 =
      max_base_pieces +
      2 * (NUM_PAWNS_PSIDE - pawns[!side] - promotions[!side]) +
      (NUM_BASE_CAPTURABLE_PIECES_PSIDE - total_base_pieces[!side]) -
      promotions[side];

  // (3) We ensure there are sufficient captured pieces and pawns to cover the
  // opposition's promotions. This only applies to side1 ensuring side0's
  // promotions are valid
  char slack = 2 * (NUM_PAWNS_PSIDE - pawns[side] - promotions[side]) +
               (NUM_BASE_CAPTURABLE_PIECES_PSIDE - total_base_pieces[side]) -
               promotions[!side];
  char pieces_lim3 = slack;
  // Pieces are assumed to be base pieces up to base_piece_lim. Otherwise by
  // considering a piece promoted where it could be base we would be decreasing
  // the number of pieces the side with the piece-in-question can have by 1 and
  // decreasing the opposition's potential promotions by 1
  if (pieces_lim3 > max_base_pieces) {
    pieces_lim3 = max_base_pieces + (char)((slack - max_base_pieces) / 2);
  }
  char max_pieces = min3(pieces_lim1, pieces_lim2, pieces_lim3);

  // We can always have none of a piece, so there are 1 + max_pieces cases.
  //
  // We also limit the number of pieces to the previous number of pieces for
  // this side. As already mentioned in count_from_pieces, previous_pieces =
  // MAX_BISHOPS_PSIDE = 10 initially. We enforce p_n >= p_(n+1) to avoid
  // counting duplicate piece sets. We later consider the # of permutations of
  // piece types to these abstract piece sets
  root->num_children =
      min(1 + max_pieces, 1 + capturable_pieces[side][rel_level - 1]);

  root->children =
      (position_node **)malloc(root->num_children * sizeof(position_node *));
  for (char i = 0; i < root->num_children; i++) {
    root->children[i] = (position_node *)malloc(sizeof(position_node));
  }

  for (int i = 0; i < root->num_children; i++) {
    capturable_pieces[side][piece_type] = i;

    char new_base_pieces = i;
    if (i > BASE_PIECES[fixed_rooks[side]][piece_type]) {
      new_base_pieces = BASE_PIECES[side][piece_type];
    }
    char new_total_base_capturable_pieces[NUM_SIDES];
    new_total_base_capturable_pieces[!side] =
        total_base_capturable_pieces[!side];
    new_total_base_capturable_pieces[side] =
        total_base_capturable_pieces[side] + new_base_pieces;

    char new_promotions[NUM_SIDES];
    new_promotions[!side] = promotions[!side];
    new_promotions[side] = promotions[side] + new_promotions;
    two_entry_wrapped_char_array new_promotions;
    new_promotions[!side] = promotions[!side];
    new_promotions[side] = promotions[side] + (i - new_base_pieces);

    // We're at a leaf
    if ((side == 1 && i == 0) || rel_level == (2 * (NUM_PIECE_TYPE)-1)) {
      uint64_t variations =
          binomials[occupiable_squares][i] *
          num_piece_type_permutations(pawns, fixed_rooks, capturable_pieces,
                                      new_total_base_capturable_pieces,
                                      new_promotions);
      mpz_init_set_ui(root->children[i]->num_positions, variations);
      mpz_add_ui(root->num_positions, root->num_positions, variations);

    } else { // We're not at a leaf
      char new_rel_level = rel_level + 1;
      if (i == 0 || new_rel_level == (NUM_PIECE_TYPES_LESS_KING + 1)) {
        new_rel_level = NUM_PIECE_TYPES_LESS_KING + 2;
      }
      count_from_pieces_helper(root->children[i], pawns, fixed_rooks,
                               capturable_pieces,
                               new_total_base_capturable_pieces, new_promotions,
                               occupiable_squares - i, rel_level);
      mpz_mul_ui(root->children[i]->num_positions,
                 root->children[i]->num_positions,
                 binomials[occupiable_squares][i]);
      mpz_add(root->num_positions, root->num_positions,
              root->children[i]->num_positions);
    }
  }

  capturable_pieces[side][piece_type] = 0;
}

void *count_from_pieces(void *arg) {
  threading_struct ts = *(threading_struct *)arg;

  char capturable_pieces[NUM_SIDES][NUM_PIECE_TYPES] = {0};
  char total_base_capturable_pieces[NUM_SIDES];
  for (char i = 0; i < NUM_SIDES; i++) {
    // MAX_BISHOPS_PSIDE = 10 is the greatest number of any piece type, since
    // we can have either 2 bishops, knights or rooks and 8 promotions.
    // count_from_pieces_helper is recursive and for a side limits the number
    // of a certain piece by the number of the previous piece (to avoid
    // duplicate abstract sides), hence why we start at the maximum of any
    // piece type. Note that the type of each piece within
    // count_from_pieces_helper is abstract. We count the number of
    // permutations of piece types to the numbers of pieces at the end of the
    // count_from_pieces_helper chain
    capturable_pieces[i][0] = MAX_BISHOPS_PSIDE;
    total_base_capturable_pieces[i] = ts.fixed_rooks[i];
  }
  char promotions[NUM_SIDES] = {0};
  count_from_pieces_helper(root->children[i], ts.pawns, ts.fixed_rooks,
                           capturable_pieces, total_base_capturable_pieces,
                           promotions, 1);
}

// Level 4 and 5 account for rooks with castling rights (which we also call
// fixed rooks) and kings. We thread from this function.
// There are 3 enpassant cases ... per side there are 3 * 3 = 9 threads used
// to build the sample structure. We have 6 threads. 3 simpler. Average
// computer 4 cores. With hyperthreading 8 threads. But since 3 will finish
// quicker, left with 3 without hyperthreading which is ideal.
void *count_from_fixed_rooks_and_kings(void *arg) {
  threading_struct ts = *(threading_struct *)arg;

  void *next_call = count_from_fixed_rooks;
  char num_children = FIXED_ROOK_SCENARIOS; // 3
  if (ts.side) {
    next_call = count_from_pieces;
    if (!ts.enpassant) {
      num_children = ts->s.fixed_rooks[0] + 1;
    }
  }

  ts.root->num_children = num_children;
  ts.root->children =
      (position_node **)malloc(root->num_children * sizeof(position_node *));
  for (char i = 0; i < root->num_children; i++) {
    ts.root->children[i] = (position_node *)malloc(sizeof(position_node));
  }

  threading_struct tstructs_out[FIXED_ROOK_SCENARIOS] = {0};
  for (char i = 0; i < num_children; i++) {
    tstructs_out[i].root = ts.root->children[i];
    tstructs_out[i].pawns = ts.pawns;
    // Always copy over fr[0] even if 0
    tstructs_out[i].fixed_rooks[0] = ts.fixed_rooks[0];
    tstructs_out[i].enpassant = ts.enpassant;
    tstructs_out[i].side = 1;
  }

  pthread_t thread_ids[FIXED_ROOK_SCENARIOS];
  // CASE 1: no castling rights
  tstructs_out[NO_CASTLING_RIGHTS].occupiable_squares =
      ts.occupiable_squares - 1; // king
  pthread_create(&thread_ids[NO_CASTLING_RIGHTS], NULL, next_call,
                 &tstructs_out[NO_CASTLING_RIGHTS]);

  // CASE 2: castling rights on one side
  if (num_children > 1) {
    tstructs_out[CASTLING_RIGHTS_ONE_SIDE].s.fixed_rooks[ts.side] = 1;
    tstructs_out[CASTLING_RIGHTS_ONE_SIDE].s.occupiable_squares =
        ts.occupiable_squares - 2;
    pthread_create(&thread_ids[CASTLING_RIGHTS_ONE_SIDE], NULL, next_call,
                   &tstructs_out[CASTLING_RIGHTS_ONE_SIDE]);
  }

  // CASE 3: castling rights on both sides
  if (num_children > 2) {
    tstructs_out[CASTLING_RIGHTS_BOTH_SIDES].s.fixed_rooks[ts.side] = 2;
    tstructs_out[CASTLING_RIGHTS_BOTH_SIDES].s.occupiable_squares =
        occupiable_squares - 3;
    pthread_create(&thread_ids[CASTLING_RIGHTS_BOTH_SIDES], NULL, next_call,
                   &tstructs_out[CASTLING_RIGHTS_BOTH_SIDES]);
  }

  for (char i = 0; i < num_children; i++) {
    pthread_join(thread_ids[i], NULL);
  }

  // king
  mpz_mul_ui(ts.root->children[NO_CASTLING_RIGHTS],
             ts.root->children[NO_CASTLING_RIGHTS], ts.occupiable_squares);

  if (num_children > 1) {
    mpz_mul_ui(ts.root->children[CASTLING_RIGHTS_ONE_SIDE]->num_positions,
               ts.root->children[CASTLING_RIGHTS_ONE_SIDE]->num_positions,
               ONE_FIXED_ROOK_VARIATIONS); // 2; fixed rook can be either side
  }

  // Account for symmetry avoiding fr[0] == fr[1]
  if (!ts.enpassant && side) {
    for (char i = 0; i < num_children - 1; i++) {
      mpz_mul_ui(ts.root->children[i]->num_positions,
                 ts.root->children[i]->num_positions, 2);
    }
  }

  mpz_init(ts.root->num_positions);
  for (char i = 0; i < num_children; i++) {
    mpz_add(ts.root->num_positions, root->num_positions,
            ts.root->children[i]->num_positions);
  }
}

// The 2nd and 3rd levels represent free (not fixed) pawns
void count_from_pawns(position_node *root, position_node *eerroot,
                      const bool side, const char occupiable_squares,
                      const bool enpassant, const char previous_pawns) {
  root->num_children = 1 + NUM_PAWNS_PSIDE;
  if (side && enpassant) {
    --root->num_children;
  } else if (!side && !enpassant) {
    root->num_children = 1 + previous_pawns;
  }

  root->children =
      (position_node **)malloc(root->num_children * sizeof(position_node *));
  if (!side && eerpointer != NULL) {
    for (char i = 0; i < root->num_children; i++) {
      root->children[i] = eerroot->children[previous_pawns]->children[i];
    }
  } else {
    for (char i = 0; i < root->num_children; i++) {
      root->children[i] = (position_node *)malloc(sizeof(position_node));
    }
    for (char i = 0; i < root->num_children; i++) {
      if (side) {
        count_from_pawns(root->children[i], eerroot, 0, occupiable_squares - i,
                         enpassant, i);
      } else {
        threading_strct t = {0};
        t.root = root->children[i];
        char pawns[NUM_SIDES];
        pawns[0] = previous_pawns;
        pawns[1] = i;
        t.pawns = pawns;
        char fixed_rooks[NUM_PAWNS] = {0};
        t.fixed_rooks = fixed_rooks;
        t.occupied_squares = occupied_squares - i;
        t.enpassant = enpassant;

        (*count_from_fixed_rooks_and_kings)(&t);

        // Accounting for symmetry
        if (i != previous_pawns) {
          mpz_mul_ui(root->children[i]->num_positions,
                     root->children[i]->num_positions, 2);
        }
      }
      mpz_mul_ui(root->children[i]->num_positions,
                 root->children[i]->num_positions,
                 binomials[occupiable_squares - (eerroot != NULL) -
                           (NUM_SIDES * BOARD_SIDE_LENGTH)][i]);
      // We subtract 16 squares from available squares because pawns can't be
      // placed on the 1st and 8th ranks
    }
  }
  mpz_init(root->num_positions);
  for (char i = 0; i < root->num_children; i++) {
    mpz_add(root->num_positions, root->num_positions,
            root->children[i]->num_positions);
  }
}

// Level 1 (0 being the root) represents en-passant scenarios
void *count_from_enpassant(position_node *root) {
  root->num_children = 3;
  root->children =
      (position_node **)malloc(root->num_children * sizeof(position_node *));
  for (char i = 0; i < root->num_children; i++) {
    root->children[i] = (position_node *)malloc(sizeof(position_node));
  }

  // CASE 1: there's no en-passant square.
  // We determine the number of pawns in side1 (within count_from_pawns)
  // before side0 even though we normally count for side0 before side1. We do
  // this because if the en-passant square is a non-edge file ([b, g]) we have
  // 2 scenarios: there are either 1 or 2 en-passant-pawn rank-adjacent
  // squares occupied by side1 pawns. We need to consider both scenarios
  // separately to avoid counting duplicate positions. We handle this
  // temporarily unoccupiable adjacency ASAP by accounting for side1 before
  // side0 pawns after which the potentially empty adjacency would then be
  // considered once again occupiable
  count_from_pawns(root->children[NO_ENPASSANT], NULL, 1, NUM_SQUARES, false,
                   NUM_PAWNS);

  // CASE 2: there's an en-passant square, and the en-passant pawn (side0's)
  // (a) has a left-rank-adjacent side1 pawn (in which case there could also
  // potentially be a right-rank-adjacent side1 pawn) or (b) is on an edge
  // file.
  //
  // Two squares are occupied by the en-passant pawn and the adjacent side1
  // pawn but there also can't be anything on the en-passant square or the
  // en-passant-pawn's starting square
  count_from_pawns(root->children[ENPASSANT_EDGE_AND_RIGHT], NULL, 1,
                   NUM_SQUARES - 4, true, NUM_PAWNS);
  mpz_mul_ui(root->children[ENPASSANT_EDGE_AND_RIGHT]->num_positions,
             root->children[ENPASSANT_EDGE_AND_RIGHT]->num_positions,
             ENPASSANT_EDGE_AND_RIGHT_VARIATIONS); // 2 + 6

  // CASE 3: there's an en-passant square, and the en-passant pawn is not on
  // an edge file and has just one rank-adjacent side1 pawn which is
  // left-rank-adjacent
  count_from_pawns(root->children[ENPASSANT_LEFT_LESS_EDGE],
                   root->children[ENPASSANT_EDGE_AND_RIGHT], 1, NUM_SQUARES - 4,
                   true, NUM_PAWNS);
  mpz_mul_ui(root->children[ENPASSANT_LEFT_LESS_EDGE]->num_positions,
             root->children[ENPASSANT_LEFT_LESS_EDGE]->num_positions,
             ENPASSANT_LEFT_LESS_EDGE_VARIATIONS); // 6

  mpz_init(root->num_positions);
  for (char i = 0; i < root->num_children; i++) {
    mpz_add(root->num_positions, root->num_positions,
            root->children[i]->num_positions);
  }
}

// Our sample space is a tree. The tree must contain at least every reachable
// position in chess, but will also contain unreachable positions as well.
// Each level of the tree represents a characteristic of a board, where boards
// with the exact same characteristics are represented by the same leaf. The
// tree doesn't store exact positions of course, that would be far too much
// memory. We abstract where exactly most pieces are. We include as much
// structure - where chessmen are or are bound within - as we would like to
// use. As potentially reachable positions which we generate from searching
// the tree we build in this file will be filtered by considering unreachable
// conditions, we can keep certain types of unreachable positions in our tree
// for later filtering where it makes sense. The tree is actually a DAG
// because certain nodes at the pawn level point to the same children.
// Ultimately the tree only has to be optimised to a point where it can be
// stored in a reasonable amount of memory and built within a reasonable
// amount of time. We build the tree in a DFS manner.
//
// Nodes in the tree are of type position_node which contains a GMP mpz_t for
// the size of the subtree rooted at the node, a char for the number of child
// nodes and an array of child nodes
void build_sample_space(position_node *root) {
  count_from_enpassant(root);
  // Our abstracted position is a tuple (side0, side1, enpassant_square,
  // move). Note that side0 and side1 don't signify white and black. We
  // enforce that only side1 can have an en-passant capture available. We also
  // enforce that if there's no en-passant square that only one of (s0, s1, m)
  // and (s1, s0, m) is accounted for in the tree. Therefore every position
  // can be cast to w/b or b/w and hence we multiply the size of the tree by 2
  // to represent the colour permutations.
  mpz_mul_ui(root->num_positions, root->num_positions, 2);
}
