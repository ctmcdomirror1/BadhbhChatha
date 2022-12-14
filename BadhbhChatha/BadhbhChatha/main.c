#include <assert.h>
#include <gmp.h>
#include <immintrin.h>
#include <inttypes.h>
#include <math.h>
#include <nmmintrin.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

//#include "bishop_colour.h"
//#include "check.h"
#include "create_tree.h"
// include "pawn_cost.h"
#include "position.h"
#include "sanity.h"
#include "search_tree.h"
#include "util.h"

#define ROOK0_BIT 0
#define ROOK1_BIT ROOK0_BIT + (BOARD_SIDE_LENGTH - 1)
#define ROOK2_BIT (BOARD_SIDE_LENGTH * (BOARD_SIDE_LENGTH - 1))
#define ROOK3_BIT ROOK2_BIT + (BOARD_SIDE_LENGTH - 1)
#define a_ASCII_decimal 97

// TODO: the move before check for the opposite side: does it have a move?

position rotate_position_across_central_rows(position p) {
  position rp;
  rp.side0isBlack = p.side0isBlack;
  rp.enpassant = rotate_bitboard_across_central_rows(p.enpassant);
  for (int i = 0; i < NUM_SIDES; i++) {
    rp.sides[i].fixed_rooks =
        rotate_bitboard_across_central_rows(p.sides[i].fixed_rooks);

    rp.sides[i].pawns = rotate_bitboard_across_central_rows(p.sides[i].pawns);

    for (int j = 0; j < NUM_PIECE_TYPES; j++) {
      rp.sides[i].pieces[j] =
          rotate_bitboard_across_central_rows(p.sides[i].pieces[j]);
    }
  }
  return rp;
}

void fen_helper(uint64_t chessmen, char symbol, char squares[NUM_SQUARES]) {
  char tz = _tzcnt_u64(chessmen);
  while (tz != NUM_SQUARES) {
    squares[tz] = symbol;
    chessmen ^= 1UL << tz;
    tz = _tzcnt_u64(chessmen);
  }
}

void print_fen(position p) {
  char squares[NUM_SQUARES];
  for (int i = 0; i < NUM_SQUARES; i++) {
    squares[i] = '1';
  }

  fen_helper(p.sides[p.side0isBlack].pawns, 'P', squares);
  fen_helper(p.sides[p.side0isBlack].pieces[BISHOP], 'B', squares);
  fen_helper(p.sides[p.side0isBlack].pieces[KNIGHT], 'N', squares);
  fen_helper(p.sides[p.side0isBlack].pieces[ROOK], 'R', squares);
  fen_helper(p.sides[p.side0isBlack].pieces[QUEEN], 'Q', squares);
  fen_helper(p.sides[p.side0isBlack].pieces[KING], 'K', squares);

  fen_helper(p.sides[!p.side0isBlack].pawns, 'p', squares);
  fen_helper(p.sides[!p.side0isBlack].pieces[BISHOP], 'b', squares);
  fen_helper(p.sides[!p.side0isBlack].pieces[KNIGHT], 'n', squares);
  fen_helper(p.sides[!p.side0isBlack].pieces[ROOK], 'r', squares);
  fen_helper(p.sides[!p.side0isBlack].pieces[QUEEN], 'q', squares);
  fen_helper(p.sides[!p.side0isBlack].pieces[KING], 'k', squares);

  for (int i = BOARD_SIDE_LENGTH - 1; i >= 0; i--) {
    for (int j = BOARD_SIDE_LENGTH - 1; j >= 0; j--) {
      printf("%c", squares[i * BOARD_SIDE_LENGTH + j]);
    }
    if (i > 0) {
      printf("/");
    }
  }

  printf(" ");
  if (p.side0isBlack) {
    printf("w ");
  } else {
    printf("b ");
  }

  bool cr0 = p.sides[0].fixed_rooks & (1UL << ROOK0_BIT);
  bool cr1 = p.sides[0].fixed_rooks & (1UL << ROOK1_BIT);
  bool cr2 = p.sides[1].fixed_rooks & (1UL << ROOK2_BIT);
  bool cr3 = p.sides[1].fixed_rooks & (1UL << ROOK3_BIT);
  if (!cr0 && !cr1 && !cr2 && !cr3) {
    printf("- ");

  } else {

    if (cr0) {
      printf("K");
    }
    if (cr1) {
      printf("Q");
    }
    if (cr2) {
      printf("k");
    }
    if (cr3) {
      printf("q");
    }
    printf(" ");
  }

  // TODO: enpassant square is square behind
  if (p.enpassant) {
    char ep = get_index_of_1st_set_bit(p.enpassant);

    char column_letter =
        a_ASCII_decimal + (BOARD_SIDE_LENGTH - 1) - (ep % BOARD_SIDE_LENGTH);
    assert(a_ASCII_decimal <= column_letter && column_letter <= 104);
    printf("%c", column_letter);

    char row_num = 1 + ((int)(ep / BOARD_SIDE_LENGTH));
    assert(row_num == 4 || row_num == 5);
    printf("%d", row_num);

  } else {
    printf("-");
  }

  printf("\n");
}

int main(int argc, char **argv) {
  compute_binomials();

  printf("Building search tree\n");
  position_node *root = malloc(sizeof(position_node));
  build_sample_space(root);
  mpf_t sample_space_size;
  mpf_init(sample_space_size);
  mpf_set_z(sample_space_size, root->num_positions);
  mpf_mul_ui(sample_space_size, sample_space_size, 2);

  gmp_printf("Number of positions in tree: %.4FE\n", sample_space_size);
  // Number of positions in tree: 2.3974E+44

  gmp_randstate_t s;
  gmp_randinit_mt(s);
  mpz_t rng;
  mpz_init(rng);

  /*
  int i;
  uint64_t successes = 0;
  long required_successes = strtol(argv[1], NULL, 10);
  for (i = 1; successes < required_successes; i++) {
    mpz_urandomm(rng, s, root->num_positions);
    position p = retrieve_position(root, rng);

    sanity_check(p);

    checking_info ci = validate_checks(p);
    char check_code = ci.code;
    if (check_code != 0) {
      printf("Invalid check code %d: ", check_code);

      p.side0isBlack = i % NUM_SIDES == 1;
      if (p.side0isBlack) {
        p = rotate_position_across_central_rows(p);
      }
      print_fen(p);
      continue;
    }

    int pawn_cost_code = validate_pawn_cost(p);
    if (pawn_cost_code != 0) {
      printf("Invalid pawn cost code %d: ", pawn_cost_code);

      p.side0isBlack = i % NUM_SIDES == 1;
      if (p.side0isBlack) {
        p = rotate_position_across_central_rows(p);
      }
      print_fen(p);
      continue;
    }

    int promotions_code = validate_num_promotions(p);
    if (promotions_code != 0) {
      printf("Invalid promotions code %d: ", promotions_code);

      p.side0isBlack = i % NUM_SIDES == 1;
      if (p.side0isBlack) {
        p = rotate_position_across_central_rows(p);
      }
      print_fen(p);
      continue;
    }

    successes++;

    printf("Valid: ");
    p.side0isBlack = i % NUM_SIDES == 1;
    if (p.side0isBlack) {
      p = rotate_position_across_central_rows(p);
    }
    print_fen(p);
  }

  mpf_t p_hat;
  mpf_init(p_hat);
  mpf_set_ui(p_hat, successes);
  mpf_div_ui(p_hat, p_hat, i);

  mpf_t z95;
  mpf_init(z95);
  mpf_set_d(z95, 1.96);

  mpf_t standard_err;
  mpf_init(standard_err);
  mpf_set_ui(standard_err, 1);
  mpf_sub(standard_err, standard_err, p_hat);
  mpf_mul(standard_err, standard_err, p_hat);
  mpf_div_ui(standard_err, standard_err, i);
  mpf_sqrt(standard_err, standard_err);
  mpf_mul(standard_err, standard_err, z95);
  mpf_mul(standard_err, standard_err, sample_space_size);

  mpf_t pbound;
  mpf_init(pbound);
  mpf_set(pbound, sample_space_size);
  mpf_mul(pbound, pbound, p_hat);

  gmp_printf("Probabilistic upperbound on the number of positions in chess is "
             "%.2FE\n +- %.2FE\n",
             pbound, standard_err);
  */

  return 0;
}
