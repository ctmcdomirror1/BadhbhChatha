#include <inttypes.h>

#include "position.h"

#define KNIGHT_ON_18_ATTACKS 0x0A1100110A

#define X_ROT_45_DEG 0x0102040810204080
#define X_ROT_NEG_45_DEG 0x8040201008040201

#define NUM_SLIDING_TYPES 3

#define EAST_ORIGIN 1
#define FILE_ORIGIN (1UL << 1)
#define WEST_ORIGIN (1UL << 2)
#define ANY_ORIGIN (1UL << 3) - 1

enum CHECK_ERROR_CODES {
  TOUCHING_KINGS = 1,
  SIDE0_CHECKED_BY_KNIGHT,                                     // 2
  SIDE0_CHECKED_BY_SLIDING_PIECE,                              // 3
  SIDE0_CHECKED_BY_PAWN,                                       // 4
  MORE_THAN_1_CHECKING_KNIGHT,                                 // 5
  MORE_THAN_1_CHECKING_PAWN,                                   // 6
  SIDE1_IN_CHECK_WHILE_ENPASSANT,                              // 7
  MORE_THAN_2_CHECKING_CHESSMEN,                               // 8
  MORE_THAN_1_CHECKING_BISHOP,                                 // 9
  CHECKED_BY_ROOK_WITH_CASTLING_RIGHTS,                        // 10
  LAYERED_SOUTH_EAST_SLIDING_ATTACK_JUST_BEHIND_CHECKING_PAWN, // 11
  LAYERED_SOUTH_WEST_SLIDING_ATTACK_JUST_BEHIND_CHECKING_PAWN, // 12
  NON_SOUTHERN_CHECKING_CHESSMAN_WITH_PAWN_ATTACK,             // 13
  CHECKED_BY_STARTING_PAWN,                                    // 14
  CHECKED_BY_PAWN_NO_PREVIOUS_SQUARE,                          // 15
  CHECKED_BY_KNIGHT_NOT_ON_PROMOTION_RANK_NO_PREVIOUS_SQUARE,  // 16
  CHECKED_BY_KNIGHT_ON_PROMOTION_RANK_NO_PREVIOUS_SQUARE,      // 17
  LAYERED_SLIDING_ATTACK_WITH_ONE_CHECKING_PIECE,
  KNIGHT_DOUBLE_CHECK_NO_INTERSECTION,
  OBTUSE_ASPECTS,
  ACUTE_SLIDING_ATTACKS_BLOCKED,
  ACUTE_SLIDING_ATTACKS_NO_INTERSECTION,
  SLIDING_ATTACK_BEHIND_CHECKING_PIECE_WHICH_JUST_MOVED
};

typedef struct checking_info {
  char code;
  char forced_capture_sq;
  char forced_promotion_sq;
  char previous_square_options;
} checking_info;

typedef struct checking_piece {
  bool type_is_queen;
  char bit;
  uint64_t ray;
  char aspect;
} checking_piece;

uint64_t get_knight_moves(char knbit);
uint64_t rotate_bitboard_across_board_center(uint64_t n);
uint64_t get_ray_exclusive(char exclusive_bit, char aspect);
checking_piece get_sliding_attack(uint64_t pot_checking_queens,
                                  uint64_t type2_bb, uint64_t blocking_chessmen,
                                  char target_bit, char aspect);
uint64_t get_checking_pawns(uint64_t pawns, uint64_t king);
char get_previous_pawn_square_possibilities(char pbit, char kbit,
                                            uint64_t occupied_squares);
char validate_checks_side0(position p);
checking_info validate_checks_side1(position p);
checking_info validate_checks(position p);
