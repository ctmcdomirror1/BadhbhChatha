#include <assert.h>
#include <inttypes.h>
#include <stdlib.h>

#include "filter_common.h"
#include "pawn_cost.h"
#include "util.h"

void test_is_kth_bit_set() {
  assert(is_kth_bit_set(5, 0));
  assert(is_kth_bit_set(8, 3));
}

void test_get_cost_reachable() {

  // -
  uint64_t pawns = 0;
  assert(get_cost(pawns, 0, 0, NULL) == 0);

  CostTestStruct *test_struct =
      (CostTestStruct *)malloc(sizeof(CostTestStruct));

  // 1 p p p p p p p p
  pawns = ROW_1;
  assert(0 == get_cost(pawns, 0, 0, test_struct));
  assert(7 == (*test_struct).left);
  assert(0 == (*test_struct).right);
  assert(28 == (*test_struct).tz_sum);
  assert(!(*test_struct).enpassant);
  // 3 p * * * * * * *
  // 2 * * * * * * * *
  // 1 p p * * * * * *
  pawns = rcb(0, 6) + rcb(0, 7) + rcb(2, 7);
  assert(2 == get_cost(pawns, 0, 0, test_struct));

  // 3 * * * * * * * p
  // 3 p p p p * * * *
  // 3 * * p p * * * *
  // 2 * * * * * * * *
  // 1 * * * * * * * p
  pawns = rcb(0, 0) + rcb(2, 4) + rcb(2, 5) + rcb(3, 4) + rcb(3, 5) +
          rcb(3, 6) + rcb(3, 7) + rcb(4, 0);
  assert(5 == get_cost(pawns, 0, 0, test_struct));

  // 6 p * * * * * * p
  // 5 * p p * p * * *
  // 4 * * * * * * * *
  // 3 * * p * * * * *
  // 2 * * p * * * * *
  // 1 * * * * p * * *
  pawns = rcb(0, 3) + rcb(1, 5) + rcb(2, 5) + rcb(4, 3) + rcb(4, 5) +
          rcb(4, 6) + rcb(5, 0) + rcb(5, 7);
  assert(6 == get_cost(pawns, 0, 0, test_struct));

  // 6 p * * * p * * *
  // 5 * * * * * * * *
  // 4 * * * * p * * *
  // 3 * * * * * * p p
  // 2 * * * p * * * *
  // 1 * * * * * * p p
  pawns = rcb(0, 0) + rcb(0, 1) + rcb(1, 4) + rcb(2, 0) + rcb(2, 1) +
          rcb(3, 3) + rcb(5, 3) + rcb(5, 7);
  assert(9 == get_cost(pawns, 0, 0, test_struct));

  // 6 p * * * * * * *
  // 5 p * * * * * * *
  // 4 p * * * * * * *
  // 3 p * * * * * * *
  // 2 p * * * * * * p
  // 1 p * * * * * * p
  pawns = rcb(0, 0) + rcb(1, 0);
  for (int i = 0; i < 6; i++) {
    pawns += rcb(i, 7);
  }
  assert(16 == get_cost(pawns, 0, 0, test_struct));

  free_cost_test_struct(test_struct);
}

void test_get_cost_with_mask_reachable() {
  CostTestStruct *test_struct =
      (CostTestStruct *)malloc(sizeof(CostTestStruct));

  // 1 p p p p p p p p
  uint64_t pawns = ROW_1;
  // 6 m m m m m m m m
  // 5 m m m m m m m m
  // 4 m m m m m m m m
  // 3 m m m m m m m m
  // 2 m m m m m m m m
  // 1 * * * * * * * *
  uint64_t mask = (-1) - ((1 << 8) - 1);
  assert(0 == get_cost(pawns, mask, 0, test_struct));
  assert(7 == (*test_struct).left);
  assert(0 == (*test_struct).right);
  assert(28 == (*test_struct).tz_sum);
  assert(!(*test_struct).enpassant);

  // 3 * p * p * p * p
  // 2 * * * * * * * *
  // 1 p * p * p * p *
  pawns = rcb(0, 1) + rcb(0, 3) + rcb(0, 5) + rcb(0, 7) + rcb(2, 0) +
          rcb(2, 2) + rcb(2, 4) + rcb(2, 6);
  // 6 m m m m m m m m
  // 5 m m m m m m m m
  // 4 m m m m m m m m
  // 3 m m m m m m m m
  // 2 m m m m m m m m
  // 1 * * * * * * * *
  mask = (-1) - ((1 << 8) - 1);
  assert(8 == get_cost(pawns, mask, 0, test_struct));
  assert(7 == (*test_struct).left);
  assert(0 == (*test_struct).right);
  assert(92 == (*test_struct).tz_sum);
  assert(!(*test_struct).enpassant);

  free_cost_test_struct(test_struct);
}

void test_get_cost_unreachable() {

  // 5 * * p * * * * *
  // 4 p * * p * * * *
  // 3 p * * * * * * *
  // 2 p p p * * * * *
  // 1 p * * * * * * *
  uint64_t pawns = rcb(0, 7) + rcb(1, 5) + rcb(1, 6) + rcb(1, 7) + rcb(2, 7) +
                   rcb(3, 4) + rcb(3, 7) + rcb(4, 5);
  assert(INFIN <= get_cost(pawns, 0, 0, NULL));

  // 2 * * * * * * * p
  // 1 * * * * * * p p
  pawns = rcb(0, 0) + rcb(0, 1) + rcb(1, 0);
  assert(INFIN <= get_cost(pawns, 0, 0, NULL));

  // 2 p * * * * * * *
  // 1 p p * * * * * *
  pawns = rcb(0, 6) + rcb(0, 7) + rcb(1, 7);
  assert(INFIN <= get_cost(pawns, 0, 0, NULL));

  // 2 * * * * p p p p
  // 1 * * * * p p p p
  pawns = 0;
  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 4; j++) {
      pawns += rcb(i, j);
    }
  }
  assert(INFIN <= get_cost(pawns, 0, 0, NULL));
}

void test_get_cost_with_mask_unreachable() {
  // 2 * p * p * p * p
  // 1 p * p * p * p *
  uint64_t pawns = rcb(0, 1) + rcb(0, 3) + rcb(0, 5) + rcb(0, 7) + rcb(1, 0) +
                   rcb(1, 2) + rcb(1, 4) + rcb(1, 6);
  uint64_t shifted_column1 = COLUMN_1 >> 8;

  // 6 p * p * p * p *
  // 5 p * p * p * p *
  // 4 p * p * p * p *
  // 3 p * p * p * p *
  // 2 p * p * p * p *
  // 1 p * p * p * p *
  uint64_t mask = (-1) - (shifted_column1 << 1) - (shifted_column1 << 3) -
                  (shifted_column1 << 5) - (shifted_column1 << 7);
  assert(INFIN <= get_cost(pawns, mask, 0, NULL));
}

void test_get_cost_with_enpassant() {
  CostTestStruct *test_struct =
      (CostTestStruct *)malloc(sizeof(CostTestStruct));

  // 3 * * * p p * * *
  // 2 * * * * * * * *
  // 1 * p * * * p p *
  uint64_t pawns = rcb(0, 1) + rcb(0, 2) + rcb(0, 6) + rcb(3, 3) + rcb(3, 4);
  uint64_t enpassant = rcb(3, 4);

  // 6 * * * * p * * *
  // 5 * * * * p * * *
  // 4 * * * * p * * *
  // 3 * * * * p * * *
  // 2 * * * * p * * *
  // 1 * * * * p * * *
  uint64_t shifted_column1 = COLUMN_1 >> 8;
  uint64_t mask = shifted_column1 << 3;

  assert(2 == get_cost(pawns, mask, enpassant, test_struct));
  assert(5 == (*test_struct).v);
  assert(2 == (int)(*test_struct).vertices[3][3]);
  assert(1 == (int)(*test_struct).vertices[3][2]);
  assert(1 == (int)(*test_struct).vertices[3][4]);
  assert(0 == (int)(*test_struct).vertices[4][4]);
  assert(INFIN == (int)(*test_struct).vertices[4][5]);
  assert((*test_struct).enpassant);

  // 5 * * * * * * p p
  // 4 * * * * * * * *
  // 3 * * p p * * * *
  // 2 * * * * p * * *
  // 1 * p * p * p * *
  pawns = rcb(0, 2) + rcb(0, 4) + rcb(0, 6) + rcb(1, 3) + rcb(2, 4) +
          rcb(2, 5) + rcb(4, 0) + rcb(4, 1);
  assert(INFIN > get_cost(pawns, 0, 0, NULL));
  assert(INFIN <= get_cost(pawns, 0, rcb(2, 5), NULL));

  // 2 p * * * * * * *
  // 1 * p * * * * * *
  pawns = rcb(0, 6) + rcb(1, 7);
  assert(INFIN == get_cost(pawns, rcb(1, 7), 0, NULL));

  // 1 * * * * * * * p
  pawns = rcb(0, 0);
  assert(INFIN == get_cost(pawns, rcb(0, 0), 0, NULL));

  // 3 * * * * * * * p
  // 2 * * * * * * * *
  // 1 * * * * * p p *
  pawns = rcb(0, 1) + rcb(1, 2) + rcb(2, 0);
  assert(2 == get_cost(pawns, rcb(2, 0), 0, NULL));

  free_cost_test_struct(test_struct);
}

void test_get_mask_for_side1() {

  // 5 * * * * * * * P
  // 4 * * * * * * * p
  // 3 * * * * * * * *
  // 2 * * * * * * * p
  uint64_t bc = rcb(2, 0) + rcb(4, 0);
  int wp = 5 * BOARD_SIDE_LENGTH;
  assert(bc == get_mask_for_side1(wp, bc));

  // 4 * * * * * * * p
  // 3 * * * * * * * P
  // 2 * * * * * * * p
  wp = 3 * BOARD_SIDE_LENGTH;
  assert(rcb(2, 0) == get_mask_for_side1(wp, bc));

  // 3 * * * P * * * *
  // 2 * * * p * * * *
  wp = rcc(3, 4);
  bc = rcb(2, 4);
  assert(rcb(2, 4) == get_mask_for_side1(wp, bc));
}

void test_get_mask_for_side0() {

  // 5 * * * * * * * P
  // 4 * * * * * * * *
  // 3 * * * * * * * P
  // 2 * * * * * * * p
  uint64_t wc = rcb(3, 0) + rcb(5, 0);
  char bp = 2 * BOARD_SIDE_LENGTH;
  assert(wc == get_mask_for_side0(bp, wc));

  // 5 * * * * * * * P
  // 4 * * * * * * * p
  // 3 * * * * * * * P
  bp = 4 * BOARD_SIDE_LENGTH;
  assert(rcb(5, 0) == get_mask_for_side0(bp, wc));

  // 3 * * * P * * * *
  // 2 * * * p * * * *
  bp = rcc(2, 4);
  wc = rcb(3, 4);
  assert(rcb(3, 4) == get_mask_for_side0(bp, wc));
}

void test_get_column_mask() {

  // 5 * * * * * * * P
  // 4 * * * * * * * p
  // 3 * * * p * * * P
  // 2 * * * P * * * p
  uint64_t white_pawns = rcb(2, 4) + rcb(3, 0) + rcb(5, 0);
  uint64_t black_pawns = rcb(2, 0) + rcb(3, 4) + rcb(4, 0);
  uint64_t masks[BOARD_SIDE_LENGTH][BOARD_SIDE_LENGTH];

  int mask_count = 0;
  uint64_t num_masks[BOARD_SIDE_LENGTH] = {0};
  for (int i = 0; i < BOARD_SIDE_LENGTH; i++) {
    int mc = get_column_mask(white_pawns, black_pawns, masks, i);
    num_masks[i] = mc;
    mask_count += mc;
  }
  assert(4 == mask_count);
  assert(4 == num_masks[0]);

  // 5 * * p * * * * *
  // 4 * * * P * * * *
  // 3 * * * P * * * *
  // 2 * * P p p * P *
  // 1 * * p * * * * *
  white_pawns = rcb(2, 1) + rcb(2, 5) + rcb(3, 4) + rcb(4, 4);
  black_pawns = rcb(1, 5) + rcb(2, 3) + rcb(2, 4) + rcb(5, 5);
  mask_count = 0;
  for (int i = 0; i < BOARD_SIDE_LENGTH; i++) {
    int mc = get_column_mask(white_pawns, black_pawns, masks, i);
    num_masks[i] = mc;
    mask_count += mc;
  }
  assert(5 == mask_count);
  assert(3 == num_masks[4]);
  assert(rcb(2, 4) == masks[4][0]);
  assert(rcb(2, 4) == masks[4][1]);
  assert((rcb(3, 4) + rcb(4, 4)) == masks[4][2]);
  assert(2 == num_masks[5]);
}

void test_find_affordable_mask() {

  // 5 * * p * * * * *
  // 4 * * * P * * * *
  // 3 * * * P * * * *
  // 2 * * P p p * P *
  // 1 * * p * * * * *
  uint64_t white_pawns = rcb(2, 1) + rcb(2, 5) + rcb(3, 4) + rcb(4, 4);
  uint64_t black_pawns = rcb(1, 5) + rcb(2, 3) + rcb(2, 4) + rcb(5, 5);
  uint64_t masks[BOARD_SIDE_LENGTH][BOARD_SIDE_LENGTH];
  int num_masks[BOARD_SIDE_LENGTH];
  for (int i = 0; i < BOARD_SIDE_LENGTH; i++) {
    num_masks[i] = get_column_mask(white_pawns, black_pawns, masks, i);
  }
  CostStruct max_costs;
  max_costs.side0 = 4;
  max_costs.side1 = 4;

  int num_calls;
  assert(find_affordable_mask(white_pawns, black_pawns, 0, masks, num_masks,
                              max_costs, 0, 0, &num_calls));

  max_costs.side0 = 1;
  max_costs.side1 = 4;
  assert(find_affordable_mask(white_pawns, black_pawns, 0, masks, num_masks,
                              max_costs, 0, 0, &num_calls));

  max_costs.side0 = 1;
  max_costs.side1 = 3;
  assert(!find_affordable_mask(white_pawns, black_pawns, 0, masks, num_masks,
                               max_costs, 0, 0, &num_calls));
  assert(6 == num_calls);
}

void test_validate_cost_helper() {
  CostStruct max_costs;
  max_costs.side0 = 7;
  max_costs.side1 = 7;

  // 6 * * * * * * p p
  // 5 * * * * * * P P
  // 4 * * * * * * p p
  // 3 p p P P * * * *
  // 2 * P * * P * * *
  // 1 * * * P p P * *
  uint64_t white_reachable = rcb(1, 2) + rcb(1, 4) + rcb(2, 3) + rcb(2, 6) +
                             rcb(3, 4) + rcb(3, 5) + rcb(5, 0) + rcb(5, 1);
  uint64_t black_reachable = rcb(1, 3) + rcb(3, 6) + rcb(3, 7) + rcb(4, 0) +
                             rcb(4, 1) + rcb(6, 0) + rcb(6, 1);
  position p = {0};
  p.sides[0].pawns = white_reachable;
  p.sides[1].pawns = black_reachable;
  assert(0 == validate_cost_helper(p, max_costs));

  // 6 * * * * * * p p
  // 5 * * * * * * P P
  // 4 * * * * * * p p
  // 3 p p P P * * * *
  // 2 * * * * P * * *
  // 1 * * * P p P * P
  uint64_t white_too_expensive = white_reachable - rcb(2, 6) + rcb(1, 0);
  p.sides[0].pawns = white_too_expensive;
  assert(SIDE0_TOO_EXPENSIVE == validate_cost_helper(p, max_costs));

  // 6 * * * * * * p p
  // 5 * * * * * * P P
  // 4 * * * * * * p p
  // 3 p p P P * * * *
  // 2 * * * * P * * p
  // 1 * * * P p P * P
  uint64_t black_too_expensive = black_reachable + rcb(2, 0);
  p.sides[1].pawns = black_too_expensive;
  assert(SIDE0_TOO_EXPENSIVE == validate_cost_helper(p, max_costs));

  // 6 * * * * * * p p
  // 5 * * * * * * P P
  // 4 * * * * * * p p
  // 3 p p P P * * * *
  // 2 * P * * P * * p
  // 1 * * * P p P * *
  p.sides[0].pawns = white_reachable;
  assert(SIDE1_TOO_EXPENSIVE == validate_cost_helper(p, max_costs));
}

int main() {
  test_is_kth_bit_set();
  test_get_cost_reachable();
  test_get_cost_with_mask_reachable();
  test_get_cost_unreachable();
  test_get_cost_with_mask_unreachable();
  test_get_cost_with_enpassant();
  test_get_mask_for_side1();
  test_get_mask_for_side0();
  test_get_column_mask();
  test_validate_cost_helper();
}
