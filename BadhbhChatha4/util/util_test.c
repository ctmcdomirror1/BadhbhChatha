#include <assert.h>
#include <stdbool.h>

#include "util.h"

// TODO: test for rcb and rcc

void test_get_index_of_1st_set_bit() {
  assert(0 == get_index_of_1st_set_bit(5));
  assert(1 == get_index_of_1st_set_bit(6));
  assert(0 == get_index_of_1st_set_bit(9));
  assert(36 == get_index_of_1st_set_bit(68719476736));
}

void test_rotate_board() {
  uint64_t bb = rcb(5, 0) + rcb(6, 1);
  uint64_t expected_bb = rcb(1, 1) + rcb(2, 0);
  assert(expected_bb == rotate_bitboard_across_central_rows(bb));
}

void test_binomials() {
  compute_binomials();

  assert(binomials[1][0] == 1);
  assert(binomials[1][1] == 1);
  assert(binomials[2][1] == 2);
  assert(binomials[10][5] == 252);
  assert(binomials[10][9] == 10);
  assert(binomials[50][8] == 536878650);
  assert(binomials[50][9] == 2505433700);
  assert(binomials[62][0] == 1);
  assert(binomials[62][7] == 491796152);
  assert(binomials[63][7] == 553270671);
  assert(binomials[63][8] == 3872894697);
  assert(binomials[64][8] == 4426165368);
  assert(binomials[64][9] == 27540584512);
  assert(binomials[64][10] == 151473214816);
}

int main() {
  test_get_index_of_1st_set_bit();
  test_rotate_board();
  test_binomials();
}
