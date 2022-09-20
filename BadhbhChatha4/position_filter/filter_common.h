#include "chess.h"

#define ROW_1 ((1UL << BOARD_SIDE_LENGTH) - 1)
#define COLUMN_1 0x0101010101010101

char get_row_num(char n);
char get_col_num(char n);
