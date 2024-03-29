#pragma once
#include <gmp.h>
#include <inttypes.h>

#include "chess.h"

#define MAX_OF_ANY_BASE_PIECE 2
#define NUM_COVERED_SETS      15
#define NUM_FUNDAMENTAL_SETS  8
#define MAX_UNIQUE_COSTS      4

#define THREE_FACTORIAL (3 * 2)
#define FOUR_FACTORIAL  (4 * THREE_FACTORIAL)

#define ENPASSANT_EDGE_AND_RIGHT_VARIATIONS ((BOARD_SIDE_LENGTH - 1) + 1)
#define ENPASSANT_LEFT_LESS_EDGE_VARIATIONS (BOARD_SIDE_LENGTH - 2)

#define ONE_FIXED_ROOK_VARIATIONS 2

#define min(a, b)     ((a) < (b) ? (a) : (b))
#define min3(a, b, c) min(min(a, b), c)

extern int coveredSet_index[MAX_OF_ANY_BASE_PIECE + 1][MAX_OF_ANY_BASE_PIECE + 1]
                           [MAX_OF_ANY_BASE_PIECE + 1][MAX_OF_ANY_BASE_PIECE + 1];
extern int fr_coveredSetIndex_fundamentalSetIndex_combIndex_comb
    [NUM_FIXED_ROOK_SCENARIOS][NUM_COVERED_SETS][NUM_FUNDAMENTAL_SETS][FOUR_FACTORIAL]
    [NUM_PIECE_TYPES_LESS_KING];
extern int fr_coveredSetIndex_permAddnCost_numPerms[NUM_FIXED_ROOK_SCENARIOS][NUM_COVERED_SETS]
                                                   [MAX_UNIQUE_COSTS];

extern uint64_t binomials[NUM_SQUARES + 1][MAX_BISHOPS_PSIDE + 1];

typedef struct position_node
{
    mpz_t                  num_positions;
    struct position_node** children;
    int                    num_children;
} position_node;

enum castling_rights_cases
{
    NO_CASTLING_RIGHTS,
    CASTLING_RIGHTS_ONE_SIDE,
    CASTLING_RIGHTS_BOTH_SIDES
};

enum enpassant_cases
{
    NO_ENPASSANT,
    ENPASSANT_EDGE_AND_RIGHT,
    ENPASSANT_LEFT_LESS_EDGE
};
