#include "annealing.h"
#include "rng.h"
#include <math.h>
#include <stddef.h>

#include <stdio.h>

extern void fill_puzzle_regions(annealing_state *puzzle_state);

static void select_neighbouring_state(annealing_state *state,
                                      carr2_u8 *new_state) {
  uint32_t region = random_uint32_t(state->random_number_generator_state) % 9;

  uint32_t some_cell_row =
      random_uint32_t(state->random_number_generator_state) % 3;
  uint32_t some_cell_column =
      random_uint32_t(state->random_number_generator_state) % 3;

  while (state->given_puzzle_positions
             ->data[((region / 3) * 3) + some_cell_row]
                   [((region % 3) * 3) + some_cell_column]) {
    some_cell_row = random_uint32_t(state->random_number_generator_state) % 3;
    some_cell_column =
        random_uint32_t(state->random_number_generator_state) % 3;
  }

  uint32_t some_other_cell_row =
      random_uint32_t(state->random_number_generator_state) % 3;
  uint32_t some_other_cell_column =
      random_uint32_t(state->random_number_generator_state) % 3;
  while (state->given_puzzle_positions
             ->data[((region / 3) * 3) + some_other_cell_row]
                   [((region % 3) * 3) + some_other_cell_column] ||
         (some_cell_row == some_other_cell_row &&
          some_cell_column == some_other_cell_column)) {
    some_other_cell_row =
        random_uint32_t(state->random_number_generator_state) % 3;
    some_other_cell_column =
        random_uint32_t(state->random_number_generator_state) % 3;
  }

  uint_fast8_t some_cell_value =
      new_state->data[((region / 3) * 3) + some_cell_row]
                     [((region % 3) * 3) + some_cell_column];
  new_state->data[((region / 3) * 3) + some_cell_row]
                 [((region % 3) * 3) + some_cell_column] =
      new_state->data[((region / 3) * 3) + some_other_cell_row]
                     [((region % 3) * 3) + some_other_cell_column];
  new_state->data[((region / 3) * 3) + some_other_cell_row]
                 [((region % 3) * 3) + some_other_cell_column] =
      some_cell_value;
}

uint32_t cost(uint_fast8_t **state) {
  uint32_t cost = 0;
  for (size_t i = 0; i < 9; i++) {
    clist_u8 found_row_nums = clist_u8_init();
    clist_u8 found_col_nums = clist_u8_init();

    for (size_t j = 0; j < 9; j++) {
      clist_u8_iter found_row_nums_iter =
          clist_u8_find(&found_row_nums, state[i][j]);
      clist_u8_iter found_row_nums_end_iter = clist_u8_end(&found_row_nums);
      if (found_row_nums_iter.ref == found_row_nums_end_iter.ref) {
        clist_u8_push_back(&found_row_nums, state[i][j]);
      } else {
        cost++;
      }

      clist_u8_iter found_col_nums_iter =
          clist_u8_find(&found_col_nums, state[j][i]);
      clist_u8_iter found_col_nums_end_iter = clist_u8_end(&found_col_nums);
      if (found_col_nums_iter.ref == found_col_nums_end_iter.ref) {
        clist_u8_push_back(&found_col_nums, state[j][i]);
      } else {
        cost++;
      }
    }

    clist_u8_drop(&found_col_nums);
    clist_u8_drop(&found_row_nums);
  }
  return cost;
}

// \f$T_0\f$ is the variance in cost for a random sample of neighbourhood
// moves.
double estimate_initial_temperature(annealing_state *state) {
  clist_u8 cost_samples = clist_u8_init();

  carr2_u8 new_state = carr2_u8_clone(*state->sudoku_puzzle_state);

  for (size_t i = 0; i < 10; i++) {
    select_neighbouring_state(state, &new_state);
    clist_u8_push_front(&cost_samples, cost(new_state.data));
  }

  uint_fast32_t total_cost = 0;
  c_foreach(cost_iter, clist_u8, cost_samples)
    total_cost += *cost_iter.ref;

  double average_cost = (double)total_cost / 10.0;

  uint_fast32_t sum = 0;
  c_foreach(cost_iter, clist_u8, cost_samples)
    sum += total_cost + pow((*cost_iter.ref - average_cost), 2);

  double variance = (double)sum / 10.0;

  clist_u8_drop(&cost_samples);

  return sqrt(variance);
}

// \f$K_{max}\f$ controls the annealing schedule. Higher \f$K_{max}\f$, slower
// anneal.
#define ANNEALING_STEP_MAX 999999

static void reheat(annealing_state *state) {
  state->number_of_state_changes = 0;
  carr2_u8_copy(state->sudoku_puzzle_state, *(state->initial_puzzle_state));
  fill_puzzle_regions(state);
  state->sudoku_puzzle_state_cost = cost(state->sudoku_puzzle_state->data);
  state->temperature = estimate_initial_temperature(state);
}

void update_annealing_state(annealing_state *state) {
  // If we reach \f$K_{max}\f$, we'll try "reheating" instead of terminating,
  // allowing a fast annealing schedule to be used.
  if (state->number_of_state_changes >= ANNEALING_STEP_MAX - 1) {
    reheat(state);
  }

  carr2_u8 new_state = carr2_u8_clone(*state->sudoku_puzzle_state);

  const double random_number_range_zero_to_one =
      ((double)random_uint32_t(state->random_number_generator_state) /
       (double)UINT32_MAX);

  // \f$s_{new} \leftarrow neighbour(s)\f$
  select_neighbouring_state(state, &new_state);

  const uint32_t cost_of_new_state = cost(new_state.data);

  const int32_t cost_difference =
      (cost_of_new_state - state->sudoku_puzzle_state_cost);

  const double acceptance_probability =
      exp((-1 * cost_difference) / state->temperature);

  // If \f$P(cost(s),cost(s_{new}), T) \geq random(0,1)\f$
  if (acceptance_probability >= random_number_range_zero_to_one ||
      cost_of_new_state == 0) {
    // \f$s \leftarrow s_{new}\f$
    memcpy(carr2_u8_data(state->sudoku_puzzle_state), carr2_u8_data(&new_state),
           carr2_u8_size(*state->sudoku_puzzle_state));
    state->sudoku_puzzle_state_cost = cost_of_new_state;
  }

  state->temperature = 1.0 - ((double)(state->number_of_state_changes + 1) /
                              (double)ANNEALING_STEP_MAX);

  carr2_u8_drop(&new_state);

  if (cost_of_new_state == 0) {
    state->annealing = false;
  }

  state->number_of_state_changes++;
}
