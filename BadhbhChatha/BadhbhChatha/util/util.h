#include <inttypes.h>
#include <stdbool.h>

#include "chess.h"

extern uint64_t binomials[NUM_SQUARES + 1][MAX_BISHOPS_PSIDE + 1];
extern char BASE_PIECE_LIMITS[NUM_PIECE_TYPES_LESS_KING];

char rcc(char row, char col);
uint64_t rcb(char row, char col);
char get_index_of_1st_set_bit(uint64_t n);
uint64_t rotate_bitboard_across_central_rows(uint64_t bb);
char num_promotions(char pieces[NUM_PIECE_TYPES_LESS_KING],
                    char base_piece_limits[NUM_PIECE_TYPES_LESS_KING]);
void compute_binomials();
