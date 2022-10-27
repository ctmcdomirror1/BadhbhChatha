#pragma once
#include "chess_constants.h"

// The four constraints we adhere to
typedef struct slack {
  int pawn_slack[NUM_SIDES];
  int chessmen_slack[NUM_SIDES];
} slack;

slack promotion_slacks(int pawns[NUM_SIDES],
                       int total_base_capturable_pieces[NUM_SIDES],
                       int promotions[NUM_SIDES]);
