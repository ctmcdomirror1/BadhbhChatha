#include <assert.h>
#include <inttypes.h>

#include "check.h"
#include "chess.h"
#include "filter_common.h"
#include "position.h"
#include "util.h"

// Could-Have-Done:
// I think it would be a good idea to have a means of defining a position as
// say a number of strings, one string per row and a function to map these
// rows to a position instead of manually configuring a position from comments.
// The way it's done in this file is error prone because at times I don't define
// everything but only the difference (I perceive) between positions

void test_knight_on_18() {
  // 4 * * * * a * a *
  // 3 * * * a * * * a
  // 2 * * * * * N * *
  // 1 * * * a * * * a
  // 0 * * * * a * a *
  uint64_t knight18 = rcb(0, 1) + rcb(0, 3) + rcb(1, 0) + rcb(1, 4) +
                      rcb(3, 0) + rcb(3, 4) + rcb(4, 1) + rcb(4, 3);
  assert(knight18 == KNIGHT_ON_18_ATTACKS);
}

void test_knight_moves() {

  // 2 * * * * * * a *
  // 1 * * * * * a * *
  // 0 * * * * * * * N
  uint64_t knight0 = rcb(1, 2) + rcb(2, 1);
  assert(knight0 == get_knight_moves(rcc(0, 0)));

  // 2 a * a * * * * *
  // 1 * * * a * * * *
  // 0 * N * * * * * *
  uint64_t knight6 = rcb(1, 4) + rcb(2, 5) + rcb(2, 7);
  assert(knight6 == get_knight_moves(rcc(0, 6)));

  // 2 * a * * * * * *
  // 1 * * a * * * * *
  // 0 N * * * * * * *
  uint64_t knight7 = rcb(1, 5) + rcb(2, 6);
  assert(knight7 == get_knight_moves(rcc(0, 7)));

  // 3 * a * * * * * *
  // 2 * * a * * * * *
  // 1 N * * * * * * *
  // 0 * * a * * * * *
  uint64_t knight15 = rcb(0, 5) + rcb(2, 5) + rcb(3, 6);
  assert(knight15 == get_knight_moves(rcc(1, 7)));

  // 6 * * * * * * a *
  // 5 * * * * * a * *
  // 4 * * * * * * * N
  // 3 * * * * * a * *
  // 2 * * * * * * a *
  uint64_t knight32 = rcb(2, 1) + rcb(3, 2) + rcb(5, 2) + rcb(6, 1);
  assert(knight32 == get_knight_moves(rcc(4, 0)));

  // 7 * * a * a * * *
  // 6 * a * * * a * *
  // 5 * * * N * * * *
  // 4 * a * * * a * *
  // 3 * * a * a * * *
  uint64_t knight44 = KNIGHT_ON_18_ATTACKS << 26;
  assert(knight44 == get_knight_moves(rcc(5, 4)));

  // 7 * * * a * * * *
  // 6 * N * * * * * *
  // 5 * * * a * * * *
  // 4 a * a * * * * *
  uint64_t knight54 = rcb(4, 5) + rcb(4, 7) + rcb(5, 4) + rcb(7, 4);
  assert(knight54 == get_knight_moves(rcc(6, 6)));

  // 7 * * * * * * * N
  // 6 * * * * * a * *
  // 5 * * * * * * a *
  uint64_t knight56 = rcb(5, 1) + rcb(6, 2);
  assert(knight56 == get_knight_moves(rcc(7, 0)));
}

void test_rotate_bitboard_across_board_center() {
  uint64_t d =
      0b1111111100000000111111001111100011110000111000001100000010000000;
  uint64_t p =
      0b0000000100000011000001110000111100011111001111110000000011111111;
  assert(d == rotate_bitboard_across_board_center(p));
}

void test_get_west_ray() {

  // 0 l l l l l l l *
  uint64_t row_0_less_0th = ROW_1 - rcb(0, 0);
  assert(row_0_less_0th == get_ray_exclusive(rcc(0, 0), 0));

  // 5 l l l l l l l *
  uint64_t row_5_less_40th = row_0_less_0th << (5 * BOARD_SIDE_LENGTH);
  assert(row_5_less_40th == get_ray_exclusive(rcc(5, 0), 0));

  // 6 l l l * * * * *
  uint64_t row_6_last_3 = rcb(6, 5) + rcb(6, 6) + rcb(6, 7);
  assert(row_6_last_3 == get_ray_exclusive(rcc(6, 4), 0));
}

void test_get_north_west_ray() {
  // 7 l * * * * * * *
  // 6 * l * * * * * *
  // 5 * * l * * * * *
  // 4 * * * l * * * *
  // 3 * * * * l * * *
  // 2 * * * * * l * *
  // 1 * * * * * * l *
  uint64_t neg_45_from_0_less_0 = X_ROT_NEG_45_DEG - rcb(0, 0);
  assert(neg_45_from_0_less_0 == get_ray_exclusive(rcc(0, 0), 1));

  // 5 l * * * * * * *
  // 4 * l * * * * * *
  // 3 * * l * * * * *
  uint64_t neg_45_from_20_less_20 = rcb(3, 5) + rcb(4, 6) + rcb(5, 7);
  assert(neg_45_from_20_less_20 == get_ray_exclusive(rcc(2, 4), 1));

  // 7 * * l * * * * *
  // 6 * * * l * * * *
  // 5 * * * * l * * *
  uint64_t neg_45_from_34_less_34 = rcb(5, 3) + rcb(6, 4) + rcb(7, 5);
  assert(neg_45_from_34_less_34 == get_ray_exclusive(rcc(4, 2), 1));
}

void test_get_north_ray() {

  // 7 * * * * * * * l
  // 6 * * * * * * * l
  // 5 * * * * * * * l
  // 4 * * * * * * * l
  // 3 * * * * * * * l
  // 2 * * * * * * * l
  // 1 * * * * * * * l
  uint64_t col_0_less_0th = COLUMN_1 - rcb(0, 0);
  assert(col_0_less_0th == get_ray_exclusive(rcc(0, 0), 2));

  // 7 * * * * l * * *
  // 6 * * * * l * * *
  // 5 * * * * l * * *
  // 4 * * * * l * * *
  // 3 * * * * l * * *
  uint64_t col_3_last_5 = (COLUMN_1 << 3) - rcb(0, 3) - rcb(1, 3) - rcb(2, 3);
  assert(col_3_last_5 == get_ray_exclusive(rcc(2, 3), 2));

  // 7 * * * * * l * *
  // 6 * * * * * l * *
  // 5 * * * * * l * *
  // 4 * * * * * l * *
  uint64_t col_2_last_4 = rcb(4, 2) + rcb(5, 2) + rcb(6, 2) + rcb(7, 2);
  assert(col_2_last_4 == get_ray_exclusive(rcc(3, 2), 2));
}

void test_get_north_east_ray() {

  // 7 * * * * * * * l
  // 6 * * * * * * l *
  // 5 * * * * * l * *
  // 4 * * * * l * * *
  // 3 * * * l * * * *
  // 2 * * l * * * * *
  // 1 * l * * * * * *
  uint64_t pos_45_from_7_less_7 = X_ROT_45_DEG - rcb(0, 7);
  assert(pos_45_from_7_less_7 == get_ray_exclusive(rcc(0, 7), 3));

  // -
  assert(0 == get_ray_exclusive(0, 3));

  // 7 * * * * l * * *
  // 6 * * * l * * * *
  uint64_t pos_45_from_45_less_45 = rcb(6, 4) + rcb(7, 3);
  assert(pos_45_from_45_less_45 == get_ray_exclusive(rcc(5, 5), 3));
}

void test_reversed_ray() {

  // 0 * l l l l l l l
  uint64_t row_0_less_7th = ROW_1 - rcb(0, 7);
  assert(row_0_less_7th == get_ray_exclusive(rcc(0, 7), 4));

  // 5 * l l l l l l l
  uint64_t row_5_less_47th = row_0_less_7th << (5 * BOARD_SIDE_LENGTH);
  assert(row_5_less_47th == get_ray_exclusive(rcc(5, 7), 4));

  // 6 * * * * l l l l
  uint64_t row_6_first_4 = rcb(6, 0) + rcb(6, 1) + rcb(6, 2) + rcb(6, 3);
  assert(row_6_first_4 == get_ray_exclusive(rcc(6, 4), 4));
}

void test_get_sliding_attacks() {

  // 3 * R Q * k B * *
  char kbit = rcc(3, 3);
  uint64_t queens = rcb(3, 5);
  uint64_t rooks = rcb(3, 6);
  uint64_t bishops = rcb(3, 2);
  checking_piece cp = get_sliding_attack(queens, rooks, bishops, kbit, 0);
  assert(cp.type_is_queen);
  assert(rcc(3, 5) == cp.bit);

  // 3 * R Q B k * * *
  kbit = rcc(3, 3);
  queens = rcb(3, 5);
  rooks = rcb(3, 6);
  bishops = rcb(3, 4);
  cp = get_sliding_attack(queens, rooks, bishops, kbit, 0);
  assert(NUM_SQUARES == cp.bit);

  // 3 * * * B k * * *
  cp = get_sliding_attack(0, 0, bishops, kbit, 0);
  assert(NUM_SQUARES == cp.bit);

  // 7 Q * k * * * R B
  kbit = rcc(7, 5);
  queens = rcb(7, 7);
  rooks = rcb(7, 1);
  bishops = rcb(7, 0);
  cp = get_sliding_attack(queens, rooks, bishops, kbit, 4);
  assert(!cp.type_is_queen);
  assert(rcc(7, 1) == cp.bit);
}

void test_get_checking_pawns() {
  // 4 k
  // 3 * P
  uint64_t king = rcb(4, 7);
  uint64_t pawns = rcb(3, 6);
  assert(pawns == get_checking_pawns(pawns, king));

  // 4 * * * * * * * k
  // 3 * * * * * * P *
  king = rcb(4, 0);
  pawns = rcb(3, 1);
  assert(pawns == get_checking_pawns(pawns, king));

  // 6 * * * k * * * *
  // 5 * * P * P * * *
  king = rcb(6, 4);
  pawns = rcb(5, 3) + rcb(5, 5);
  assert(pawns == get_checking_pawns(pawns, king));
}

void test_get_previous_pawn_square_possibilities() {
  // 7 * * k Q * * * *
  // 6 * * p * * * * *
  uint64_t occupied_squares = rcb(6, 5);
  assert(1 == get_previous_pawn_square_possibilities(rcc(7, 4), rcc(7, 5),
                                                     occupied_squares));

  // 7 * * k Q * * * *
  // 6 * * * * p * * *
  occupied_squares = rcb(6, 3);
  assert(4 == get_previous_pawn_square_possibilities(rcc(7, 4), rcc(7, 5),
                                                     occupied_squares));

  // 7 * * k Q * * * *
  // 6 * * p * p * * *
  occupied_squares = rcb(6, 3) + rcb(6, 5);
  assert(0 == get_previous_pawn_square_possibilities(rcc(7, 4), rcc(7, 5),
                                                     occupied_squares));

  // 7 k * * Q * * * *
  // 6 * * * * * * * *
  occupied_squares = 0;
  assert(7 == get_previous_pawn_square_possibilities(rcc(7, 4), rcc(7, 7),
                                                     occupied_squares));
}

void test_touching_kings() {
  // 5 * * * * k * * *
  // 4 * * * K * * * *
  position p = {0};
  p.sides[1].pieces[KING] = rcb(5, 3);
  p.sides[0].pieces[KING] = rcb(4, 4);
  assert(TOUCHING_KINGS == validate_checks(p).code);

  // 1 * * * * * * * K
  // 0 * * * * * * * k
  p.sides[0].pieces[KING] = rcb(1, 0);
  p.sides[1].pieces[KING] = rcb(0, 0);
  assert(TOUCHING_KINGS == validate_checks(p).code);

  // 1 * * * * * K * *
  // 0 * * * * * * * k
  p.sides[0].pieces[KING] = rcb(1, 2);
  assert(0 == validate_checks(p).code);

  // 3 * * * K * * * *
  // 2 * * * * * * * *
  // 1 * * * k * * * *
  p.sides[0].pieces[KING] = rcb(3, 4);
  p.sides[1].pieces[KING] = rcb(1, 4);
  assert(0 == validate_checks(p).code);
}

void test_side0_is_not_in_check() {

  // 0 * * * * * * * K
  position p = {0};
  p.sides[0].pieces[KING] = rcb(0, 0);
  assert(0 == validate_checks_side0(p));

  // 2 * * * * * * b *
  // 1 r * * * * * * p
  // 0 * * * * * n * K
  p.sides[1].pieces[ROOK] = rcb(1, 7);
  p.sides[1].pieces[BISHOP] = rcb(2, 1);
  p.sides[1].pieces[KNIGHT] = rcb(0, 2);
  p.sides[1].pawns = rcb(1, 0);
  assert(0 == validate_checks_side0(p));
}

void test_side0_is_in_check() {

  // 1 * * * * * n * *
  // 0 * * * * * * * K
  position p = {0};
  p.sides[0].pieces[KING] = rcb(0, 0);
  p.sides[1].pieces[KNIGHT] = rcb(1, 2);
  assert(SIDE0_CHECKED_BY_KNIGHT == validate_checks_side0(p));

  // 2 * * * * * b * *
  // 1 * * * * * * * *
  // 0 * * * K * * * *
  p.sides[0].pieces[KING] = rcb(0, 4);
  p.sides[1].pieces[KNIGHT] = 0;
  p.sides[1].pieces[BISHOP] = rcb(2, 2);
  assert(SIDE0_CHECKED_BY_SLIDING_PIECE == validate_checks_side0(p));

  // 2 * * * * * * * *
  // 1 * * p * * * * *
  // 0 * * * K * * * *
  p.sides[1].pieces[BISHOP] = 0;
  p.sides[1].pawns = rcb(1, 5);
  assert(SIDE0_CHECKED_BY_PAWN == validate_checks_side0(p));
}

void test_side1_is_not_in_check() {

  // 0 * * * * * * * k
  position p = {0};
  p.sides[1].pieces[KING] = rcb(0, 0);
  assert(0 == validate_checks_side1(p).code);

  // 2 * * * * * * B *
  // 1 R * * * * * * P
  // 0 * * * * * N * k
  p.sides[0].pieces[ROOK] = rcb(1, 7);
  p.sides[0].pieces[BISHOP] = rcb(2, 1);
  p.sides[0].pieces[KNIGHT] = rcb(0, 2);
  p.sides[0].pawns = rcb(1, 0);
  assert(0 == validate_checks_side1(p).code);
}

void test_valid_one_checking_chessman() {
  // 3 * * * * * * * k
  // 2 * * * * * * P *
  position p = {0};
  p.sides[1].pieces[KING] = rcb(3, 0);
  p.sides[0].pawns = rcb(2, 1);
  assert(0 == validate_checks_side1(p).code);

  // 4 B * * * * * * *
  // 3 * * * * * * * *
  // 2 * * k * * * * *
  p.sides[0].pawns = 0;
  p.sides[1].pieces[KING] = rcb(2, 5);
  p.sides[0].pieces[BISHOP] = rcb(4, 7);
  assert(0 == validate_checks_side1(p).code);

  // 3 N * * * * * * *
  // 2 * * k * * * * *
  p.sides[0].pieces[BISHOP] = 0;
  p.sides[0].pieces[KNIGHT] = rcb(3, 7);
  assert(0 == validate_checks_side1(p).code);

  // 2 * * k * * * * R
  p.sides[0].pieces[KNIGHT] = 0;
  p.sides[0].pieces[ROOK] = rcb(2, 0);
  assert(0 == validate_checks_side1(p).code);

  // 4 * * Q * * * * *
  // 3 * * * * * * * *
  // 2 * * k * * * * *
  p.sides[0].pieces[ROOK] = 0;
  p.sides[0].pieces[QUEEN] = rcb(4, 5);
  assert(0 == validate_checks_side1(p).code);

  // 4 * * * * k * * *
  // 3 * * * P * * * *
  // 2 * * * * * * * *
  // 1 * B * * * * * *
  p.sides[0].pieces[QUEEN] = 0;
  p.sides[1].pieces[KING] = rcb(4, 3);
  p.sides[0].pawns = rcb(3, 4);
  p.sides[0].pieces[BISHOP] = rcb(1, 6);
  checking_info ci = validate_checks_side1(p);
  assert(0 == ci.code);
  assert(rcb(3, 4) == ci.checking_pawn);
  assert(4 == ci.checking_pawn_origin_options);

  // 4 * * * * k * * *
  // 3 * * * P * * * *
  // 3 * * * * p * * *
  p.sides[0].pieces[BISHOP] = 0;
  p.sides[1].pawns = rcb(2, 3);
  ci = validate_checks_side1(p);
  assert(0 == ci.code);
  assert(rcb(3, 4) == ci.checking_pawn);
  assert(6 == ci.checking_pawn_origin_options);

  // 6 * * * * * * * *
  // 5 * * k * * * * *
  // 4 N * * * * * * *
  // 3 * * p * * * * *
  // 2 * p * * * * * *
  p.sides[1].pawns = 0;
  p.sides[0].pieces[KNIGHT] = rcb(4, 7);
  p.sides[1].pieces[KING] = rcb(5, 5);
  p.sides[1].pawns = rcb(2, 6) + rcb(3, 5);
  ci = validate_checks_side1(p);
  assert(0 == ci.code);

  // 7 * * * N * * * *
  // 6 * k * * * * * *
  // 5 * * p * p * * *
  p.sides[0].pieces[KNIGHT] = rcb(7, 4);
  p.sides[1].pieces[KING] = rcb(6, 6);
  p.sides[1].pawns = rcb(5, 3) + rcb(5, 5);
  ci = validate_checks_side1(p);
  assert(0 == ci.code);

  // 7 * * * N * * * *
  // 6 * k * * * p * *
  // 5 * * p * p * * *
  p.sides[0].pieces[KNIGHT] = rcb(7, 4);
  p.sides[1].pieces[KING] = rcb(6, 6);
  p.sides[1].pawns = rcb(5, 3) + rcb(5, 5) + rcb(6, 2);
  ci = validate_checks_side1(p);
  assert(0 == ci.code);
  assert((1UL << rcc(7, 4)) == ci.promoted_piece);
  assert(7 == ci.promotion_origin_options);
}

void test_valid_checking_combinations() {
  // 4 * B * * N * * *
  // 3 * * * * * * * *
  // 2 * * * k * * * *
  position p = {0};
  p.sides[1].pieces[KING] = rcb(2, 4);
  p.sides[0].pieces[BISHOP] = rcb(4, 6);
  p.sides[0].pieces[KNIGHT] = rcb(4, 3);
  assert(0 == validate_checks_side1(p).code);

  // 5 * * * R * * * *
  // 4 * * * * * * * *
  // 3 * N * * * * * *
  // 2 * * * k * * * *
  p.sides[0].pieces[BISHOP] = 0;
  p.sides[0].pieces[KNIGHT] = rcb(3, 6);
  p.sides[0].pieces[ROOK] = rcb(5, 4);
  assert(0 == validate_checks_side1(p).code);

  // 6 * * * R * * * *
  // 5 * * * R * * * *
  // 4 * * * * * * * *
  // 3 * N * * * * * *
  // 2 * * * k * * * *
  p.sides[0].pieces[BISHOP] = 0;
  p.sides[0].pieces[KNIGHT] = rcb(3, 6);
  p.sides[0].pieces[ROOK] = rcb(5, 4) + rcb(6, 4);
  assert(0 == validate_checks_side1(p).code);

  // 5 * * * Q * * * *
  // 4 * * * * * * * *
  // 3 * N * * * * * *
  // 2 * * * k * * * *
  p.sides[0].pieces[ROOK] = 0;
  p.sides[0].pieces[KNIGHT] = rcb(3, 6);
  p.sides[0].pieces[QUEEN] = rcb(5, 4);
  assert(0 == validate_checks_side1(p).code);

  // 4 * B * * * * * *
  // 3 * * * R * * * *
  // 2 * * * k * * * *
  p.sides[0].pieces[KNIGHT] = 0;
  p.sides[0].pieces[QUEEN] = 0;
  p.sides[0].pieces[ROOK] = rcb(3, 4);
  p.sides[0].pieces[BISHOP] = rcb(4, 6);
  assert(0 == validate_checks_side1(p).code);

  // 5 * * * Q * * * *
  // 4 * * * * * * * *
  // 3 * * B * * * * *
  // 2 * * * k * * * *
  p.sides[0].pieces[ROOK] = 0;
  p.sides[0].pieces[BISHOP] = rcb(3, 5);
  p.sides[0].pieces[QUEEN] = rcb(5, 4);
  assert(0 == validate_checks_side1(p).code);

  // 4 * Q * * * * * *
  // 3 * * * R * * * *
  // 2 * * * k * * * *
  p.sides[0].pieces[BISHOP] = 0;
  p.sides[0].pieces[ROOK] = rcb(3, 4);
  p.sides[0].pieces[QUEEN] = rcb(4, 6);
  assert(0 == validate_checks_side1(p).code);
}

void test_invalid_checks() {
  // 4 * * k * * * * *
  // 3 N * * * N * * *
  position p = {0};
  p.sides[1].pieces[KING] = rcb(4, 5);
  p.sides[0].pieces[KNIGHT] = rcb(3, 3) + rcb(3, 7);
  assert(MORE_THAN_1_CHECKING_KNIGHT == validate_checks_side1(p).code);

  // 4 * * k * * * * *
  // 3 * P * P * * * *
  p.sides[0].pieces[KNIGHT] = 0;
  p.sides[0].pawns = rcb(3, 4) + rcb(3, 6);
  assert(MORE_THAN_1_CHECKING_PAWN == validate_checks_side1(p).code);

  // 1 * B * B * * * *
  // 0 * * k * * * * *
  p.sides[0].pawns = 0;
  p.sides[1].pieces[KING] = rcb(0, 5);
  p.sides[0].pieces[BISHOP] = rcb(1, 4) + rcb(1, 6);
  assert(MORE_THAN_1_BISHOP_OR_MORE_THAN_1_ROOK ==
         validate_checks_side1(p).code);

  // 0 R * k * * * * R
  p.sides[0].pieces[BISHOP] = 0;
  p.sides[0].pieces[ROOK] = rcb(0, 0) + rcb(0, 7);
  assert(MORE_THAN_1_SLIDING_ATTACK == validate_checks_side1(p).code);

  // 4 * * k * * * * Q
  // 3 * * * * * * * *
  // 2 * * * * Q * * *
  p.sides[1].pieces[KING] = rcb(4, 5);
  p.sides[0].pieces[QUEEN] = rcb(4, 0) + rcb(2, 3);
  assert(MORE_THAN_1_SLIDING_ATTACK == validate_checks_side1(p).code);

  // 4 * * k * * * * Q
  // 3 * * * * N * * *
  // 2 * * * * B * * *
  p.sides[1].pieces[KING] = rcb(4, 5);
  p.sides[0].pieces[BISHOP] = rcb(2, 3);
  p.sides[0].pieces[KNIGHT] = rcb(3, 3);
  p.sides[0].pieces[QUEEN] = rcb(4, 0);
  assert(MORE_THAN_2_CHECKING_CHESSMEN == validate_checks_side1(p).code);

  // 2 * * * * * * * k
  // 1 * * * * * * * *
  // 0 * * * * * * * R
  p.sides[0].pieces[BISHOP] = 0;
  p.sides[0].pieces[KNIGHT] = 0;
  p.sides[0].pieces[QUEEN] = 0;
  p.sides[1].pieces[KING] = rcb(2, 0);
  p.sides[0].pieces[ROOK] = rcb(0, 0);
  p.sides[0].fixed_rooks = p.sides[0].pieces[ROOK];
  assert(CHECKED_BY_ROOK_WITH_CASTLING_RIGHTS == validate_checks_side1(p).code);

  // 0 R * k * * * * *
  p.sides[1].pieces[KING] = rcb(0, 5);
  p.sides[0].pieces[ROOK] = rcb(0, 7);
  p.sides[0].fixed_rooks = p.sides[0].pieces[ROOK];
  assert(CHECKED_BY_ROOK_WITH_CASTLING_RIGHTS == validate_checks_side1(p).code);

  // 3 * * * * k * * *
  // 2 * * * * * P * *
  // 1 * * * * * * B *
  p.sides[0].pieces[ROOK] = 0;
  p.sides[0].fixed_rooks = 0;
  p.sides[1].pieces[KING] = rcb(3, 3);
  p.sides[0].pawns = rcb(2, 2);
  p.sides[0].pieces[BISHOP] = rcb(1, 1);
  assert(LAYERED_SOUTH_EAST_SLIDING_ATTACK_JUST_BEHIND_CHECKING_PAWN ==
         validate_checks_side1(p).code);

  // 3 * * * k * * * *
  // 2 * * P * * * * *
  // 1 * B * * * * * *
  p.sides[1].pieces[KING] = rcb(3, 4);
  p.sides[0].pawns = rcb(2, 5);
  p.sides[0].pieces[BISHOP] = rcb(1, 6);
  assert(LAYERED_SOUTH_WEST_SLIDING_ATTACK_JUST_BEHIND_CHECKING_PAWN ==
         validate_checks_side1(p).code);

  // 2 k
  // 1 * P
  p.sides[0].pieces[BISHOP] = 0;
  p.sides[1].pieces[KING] = rcb(2, 7);
  p.sides[0].pawns = rcb(1, 6);
  assert(CHECKED_BY_STARTING_PAWN == validate_checks_side1(p).code);

  // 3 k * *
  // 2 * P *
  // 1 R R R
  p.sides[1].pieces[BISHOP] = 0;
  p.sides[1].pieces[KING] = rcb(3, 7);
  p.sides[0].pawns = rcb(2, 6);
  p.sides[0].pieces[ROOK] = rcb(1, 5) + rcb(1, 6) + rcb(1, 7);
  assert(CHECKED_BY_PAWN_NO_PREVIOUS_SQUARE == validate_checks_side1(p).code);

  // 3 * k
  // 2 P *
  // 1 R P R
  p.sides[1].pieces[KING] = rcb(3, 6);
  p.sides[0].pawns = rcb(1, 6) + rcb(2, 7);
  p.sides[0].pieces[ROOK] = rcb(1, 5) + rcb(1, 7);
  assert(CHECKED_BY_PAWN_NO_PREVIOUS_SQUARE == validate_checks_side1(p).code);

  // 7 * * * * * * * N
  // 6 * * * * * k p p
  p.sides[1].pieces[KING] = rcb(6, 2);
  p.sides[1].pawns = rcb(6, 0) + rcb(6, 1);
  p.sides[0].pieces[KNIGHT] = rcb(7, 0);
  assert(CHECKED_BY_KNIGHT_ON_PROMOTION_RANK_NO_PREVIOUS_SQUARE);

  // 7 N * * * * * * *
  // 6 p p k * * * * *
  p.sides[1].pieces[KING] = rcb(6, 5);
  p.sides[1].pawns = rcb(6, 6) + rcb(6, 7);
  p.sides[0].pieces[KNIGHT] = rcb(7, 7);
  assert(CHECKED_BY_KNIGHT_ON_PROMOTION_RANK_NO_PREVIOUS_SQUARE);

  // 7 * N * * * * * *
  // 6 p p p k * * * *
  p.sides[1].pieces[KING] = rcb(6, 4);
  p.sides[1].pawns = rcb(6, 5) + rcb(6, 6) + rcb(6, 7);
  p.sides[0].pieces[KNIGHT] = rcb(7, 6);
  assert(CHECKED_BY_KNIGHT_ON_PROMOTION_RANK_NO_PREVIOUS_SQUARE);

  // 6 * k
  // 5 * * p
  // 4 N
  // 3 * * p
  // 2 * p
  p.sides[1].pieces[KING] = rcb(6, 6);
  p.sides[1].pawns = rcb(2, 6) + rcb(3, 5) + rcb(5, 5);
  p.sides[0].pieces[KNIGHT] = rcb(4, 7);
  assert(CHECKED_BY_KNIGHT_NOT_ON_PROMOTION_RANK_NO_PREVIOUS_SQUARE ==
         validate_checks_side1(p).code);

  // 0 * * * * * Q R k
  p.sides[1].pawns = 0;
  p.sides[0].pawns = 0;
  p.sides[1].pieces[KNIGHT] = 0;
  p.sides[1].pieces[KING] = rcb(0, 0);
  p.sides[0].pieces[ROOK] = rcb(0, 1);
  p.sides[0].pieces[QUEEN] = rcb(0, 2);
  assert(LAYERED_SLIDING_ATTACK_WITH_ONE_CHECKING_PIECE ==
         validate_checks_side1(p).code);

  // 2 * * * * * B * *
  // 1 * * * * * * Q *
  // 0 * * * * * * * k
  p.sides[0].pieces[ROOK] = 0;
  p.sides[0].pieces[BISHOP] = rcb(2, 2);
  p.sides[0].pieces[QUEEN] = rcb(1, 1);
  assert(LAYERED_SLIDING_ATTACK_WITH_ONE_CHECKING_PIECE ==
         validate_checks_side1(p).code);

  // 3 * * k * * * * *
  // 2 * * * P * * * *
  // 1 * N * * * * * *
  p.sides[1].pieces[KING] = rcb(3, 5);
  p.sides[0].pawns = rcb(2, 4);
  p.sides[0].pieces[KNIGHT] = rcb(1, 6);
  assert(MORE_THAN_1_CHECKING_CHESSMAN_WITH_PAWN_ATTACK ==
         validate_checks_side1(p).code);

  // 3 * * k * * * * *
  // 2 * * * * * * * *
  // 1 B N * * * * * *
  p.sides[0].pawns = 0;
  p.sides[0].pieces[BISHOP] = rcb(1, 7);
  assert(KNIGHT_DOUBLE_CHECK_NO_INTERSECTION == validate_checks_side1(p).code);

  // 3 * R k * * * * *
  // 2 * * * Q * * * *
  p.sides[0].pieces[KNIGHT] = 0;
  p.sides[0].pieces[BISHOP] = 0;
  p.sides[0].pieces[ROOK] = rcb(3, 6);
  p.sides[0].pieces[QUEEN] = rcb(2, 4);
  assert(OBTUSE_ASPECTS == validate_checks_side1(p).code);

  // 3 * R * k * * * *
  // 2 * n * * * * * *
  // 1 * * * * * * * *
  // 0 Q * * * * * * *
  p.sides[1].pieces[KING] = rcb(3, 4);
  p.sides[0].pieces[ROOK] = rcb(3, 6);
  p.sides[0].pieces[QUEEN] = rcb(0, 7);
  p.sides[1].pieces[KNIGHT] = rcb(2, 6);
  assert(ACUTE_SLIDING_ATTACKS_BLOCKED == validate_checks_side1(p).code);

  // 3 * R * k * * * *
  // 2 * * * * * * * *
  // 2 * Q * * * * * *
  p.sides[1].pieces[KNIGHT] = 0;
  p.sides[0].pieces[QUEEN] = rcb(2, 5);
  assert(ACUTE_SLIDING_ATTACKS_NO_INTERSECTION ==
         validate_checks_side1(p).code);

  // 3 R R * k * * * *
  // 2 * * * * * * * *
  // 1 * * * * * * * *
  // 0 Q * * * * * * *
  p.sides[0].pieces[QUEEN] = rcb(0, 7);
  p.sides[0].pieces[ROOK] += rcb(3, 7);
  assert(SLIDING_ATTACK_BEHIND_CHECKING_PIECE_WHICH_JUST_MOVED ==
         validate_checks_side1(p).code);
}

void test_empirical0() {

  // 7 * * * R * * B k
  // 6 N N * * P * * *
  // 5 P q * * * * p Q
  // 4 * b p * * * * *
  // 3 * * * * P * * n
  // 2 R r * n p q b *
  // 1 * p P B K * * P
  // 0 * * r * N b Q *
  position p = {0};
  p.sides[0].pieces[KING] = rcb(1, 3);
  p.sides[0].pawns = rcb(1, 0) + rcb(1, 5) + rcb(3, 3) + rcb(5, 7) + rcb(6, 4);
  p.sides[0].pieces[BISHOP] = rcb(1, 4) + rcb(7, 1);
  p.sides[0].pieces[KNIGHT] = rcb(0, 3) + rcb(6, 6) + rcb(6, 7);
  p.sides[0].pieces[ROOK] = rcb(2, 7);
  p.sides[0].pieces[QUEEN] = rcb(0, 1) + rcb(5, 0);

  p.sides[1].pieces[KING] = rcb(7, 0);
  p.sides[1].pawns = rcb(1, 6) + rcb(2, 3) + rcb(4, 5) + rcb(5, 1);
  p.sides[1].pieces[BISHOP] = rcb(0, 2) + rcb(2, 1) + rcb(4, 6);
  p.sides[1].pieces[KNIGHT] = rcb(2, 4) + rcb(3, 0);
  p.sides[1].pieces[ROOK] = rcb(0, 5) + rcb(2, 6);
  p.sides[1].pieces[QUEEN] = rcb(2, 2) + rcb(5, 6);
  assert(SIDE0_CHECKED_BY_SLIDING_PIECE == validate_checks(p).code);
}

void test_empirical1() {

  // 7 B * * * * * K *
  // 6 * * Q B * b r *
  // 5 * * * * N * * *
  // 4 * * * * * * * *
  // 3 * * * * * * * *
  // 2 * * * * * * * *
  // 1 * * * * * * * *
  // 0 k * * * * * * *
  position p = {0};
  p.sides[1].pieces[KING] = rcb(7, 1);
  p.sides[1].pieces[KNIGHT] = rcb(5, 3);
  p.sides[1].pieces[BISHOP] = rcb(6, 4) + rcb(7, 7);
  p.sides[1].pieces[QUEEN] = rcb(6, 5);

  p.sides[0].pieces[KING] = rcb(0, 7);
  p.sides[0].pieces[ROOK] = rcb(6, 1);
  p.sides[0].pieces[BISHOP] = rcb(6, 2);
  assert(ACUTE_SLIDING_ATTACKS_NO_INTERSECTION == validate_checks(p).code);

  // 1 * * * * * B R *
  // 0 K * * * * * k *
  position p1 = {0};
  p1.sides[1].pieces[KING] = rcb(0, 1);

  p1.sides[0].pieces[KING] = rcb(0, 7);
  p1.sides[0].pieces[ROOK] = rcb(1, 1);
  p1.sides[0].pieces[BISHOP] = rcb(1, 2);
  assert(ACUTE_SLIDING_ATTACKS_NO_INTERSECTION == validate_checks(p1).code);
}

void test_empirical2() {
  // 7 k * * * * * * *
  // 6 * * * * * * p *
  // 5 * * * * * * * K
  position p = {0};
  p.sides[0].pieces[KING] = rcb(5, 0);

  p.sides[1].pawns = rcb(6, 1);
  p.sides[1].pieces[KING] = rcb(7, 7);
  assert(SIDE0_CHECKED_BY_PAWN == validate_checks(p).code);
}

int main() {

  test_rotate_bitboard_across_board_center();
  test_knight_on_18();
  test_knight_moves();
  test_get_west_ray();
  test_get_north_west_ray();
  test_get_north_ray();
  test_get_north_east_ray();
  test_reversed_ray();
  test_get_sliding_attacks();
  test_get_checking_pawns();
  test_get_previous_pawn_square_possibilities();

  test_touching_kings();

  test_side0_is_not_in_check();
  test_side0_is_in_check();

  test_side1_is_not_in_check();
  test_valid_one_checking_chessman();
  test_valid_checking_combinations();
  test_invalid_checks();

  test_empirical0();
  test_empirical1();
  test_empirical2();

  return 0;
}
