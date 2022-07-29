#pragma once

#include <stdbool.h>
#include <inttypes.h>

struct annealing_state {
  bool annealing;
  double temperature;
  uint8_t **sudoku_puzzle_state;
  uint8_t **given_puzzle_positions;
  uint64_t number_of_state_changes;
  uint64_t number_of_state_changes_with_higher_cost;
};

typedef struct annealing_state annealing_state;

double estimate_initial_temperature(uint8_t **sudoku_puzzle,
                                    uint8_t **given_puzzle_positions);

void update_annealing_state(annealing_state* state);
