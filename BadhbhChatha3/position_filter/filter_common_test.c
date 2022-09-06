#include <assert.h>

void test_get_row_num() {
  assert(0 == get_row_num(1));
  assert(0 == get_row_num(7));
  assert(1 == get_row_num(8));
  assert(5 == get_row_num(47));
}

void test_get_col_num() {
  assert(0 == get_col_num(0));
  assert(1 == get_col_num(1));
  assert(7 == get_col_num(7));
  assert(0 == get_col_num(8));
  assert(7 == get_col_num(47));
}

int main() {
  test_get_row_num();
  test_get_col_num();
}
