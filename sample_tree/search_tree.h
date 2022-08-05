#include <gmp.h>

#include "position.h"
#include "tree_common.h"

#define EDGE_ROW_MASK                                                          \
  -1 - ((1UL << (7 * BOARD_SIDE_LENGTH)) - 1) + ((1UL << BOARD_SIDE_LENGTH) - 1)

position retrieve_position(position_node *root, mpz_t rng);
