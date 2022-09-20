#include <inttypes.h>
#include <stdbool.h>

#include "chess.h"
#include "position.h"

#define INFIN (uint64_t)100

enum PAWN_COST_ERROR_CODES { SIDE0_TOO_EXPENSIVE = 1, SIDE1_TOO_EXPENSIVE };

typedef struct CostTestStruct {
  bool enpassant;
  char left;
  char right;
  int tz_sum;
  char v;
  double **vertices;
} CostTestStruct;

typedef struct CostStruct {
  char side0;
  char side1;
} CostStruct;

bool is_kth_bit_set(uint64_t n, char k);

uint64_t get_mask_for_side0(char tz, uint64_t column);
uint64_t get_mask_for_side1(char lz, uint64_t column);

void free_cost_test_struct(CostTestStruct *test_struct);

uint64_t get_cost(uint64_t pawns, uint64_t mask, uint64_t enpassant,
                  CostTestStruct *test_struct);

int get_column_mask(uint64_t white, uint64_t black,
                    uint64_t masks[BOARD_SIDE_LENGTH][BOARD_SIDE_LENGTH],
                    int col);

char find_affordable_mask(uint64_t side0, uint64_t side1, uint64_t enpassant,
                          uint64_t masks[BOARD_SIDE_LENGTH][BOARD_SIDE_LENGTH],
                          int num_masks[BOARD_SIDE_LENGTH],
                          CostStruct max_costs, uint64_t mask,
                          unsigned char col, int *num_calls);

char validate_cost_helper(position p, CostStruct max_costs);
char validate_pawn_cost(position p);
