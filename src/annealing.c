#include "annealing.h"

// \f$T_0\f$ is the variance in cost for a random sample of neighbourhood
// moves.
double estimate_initial_temperature(uint8_t **sudoku_puzzle,
                                    uint8_t **given_puzzle_positions) {
  (void)sudoku_puzzle;
  (void)given_puzzle_positions;
  return 1.0;
}

#define ANNEALING_STEP_MAX 9999999

void update_annealing_state(annealing_state *state) {
  if (state->number_of_state_changes >= ANNEALING_STEP_MAX) {
    state->annealing = false;
    return;
  }

  uint64_t number_of_state_changes_per_temperature = 0;
  for (uint64_t i = 0; i < 9; i++) {
    for (uint64_t j = 0; j < 9; j++) {
      number_of_state_changes_per_temperature +=
          (state->given_puzzle_positions[i][j] != 0);
    }
  }

  state->annealing = false;

  state->number_of_state_changes++;
}
