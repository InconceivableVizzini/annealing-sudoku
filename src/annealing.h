#pragma once

#include <inttypes.h>
#include <stdbool.h>

#define i_val uint_fast8_t
#define i_tag u8
#include <stc/carr2.h>

#define i_val uint_fast8_t
#define i_tag u8
#include <stc/clist.h>

struct annealing_state {
  uint32_t random_number_generator_state[4];
  double temperature;
  uint64_t number_of_state_changes;
  carr2_u8 *initial_puzzle_state;
  carr2_u8 *sudoku_puzzle_state;
  carr2_u8 *given_puzzle_positions;
  uint32_t sudoku_puzzle_state_cost;
  bool annealing;
};

typedef struct annealing_state annealing_state;

double estimate_initial_temperature(annealing_state *state);

void update_annealing_state(annealing_state *state);

uint32_t cost(uint_fast8_t **state);
