#include "annealing.h"
#include "rng.h"
#include <math.h>
#include <stddef.h>

#include <stdio.h>

// \f$K_{max}\f$ controls the annealing schedule. Higher \f$K_{max}\f$, slower
// anneal.
#define ANNEALING_STEP_MAX 999999

extern void fill_puzzle_regions(annealing_state *puzzle_state);

static void select_neighbouring_state(annealing_state *state,
                                      carr2_u8 *new_state) {
  const uint32_t region =
      random_uint32_t(state->random_number_generator_state) % 9;

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

  const uint_fast8_t some_cell_value =
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

// If a solution was not found quickly with the fast annealing schedule reset
// the annealing state to a new random initial configuration.
static void reheat(annealing_state *state) {
  state->number_of_state_changes = 0;
  carr2_u8_copy(state->sudoku_puzzle_state, *(state->initial_puzzle_state));
  fill_puzzle_regions(state);
  state->sudoku_puzzle_state_cost = cost(state->sudoku_puzzle_state->data);
  state->temperature = 1.0;
}

void update_annealing_state(annealing_state *state) {
  // If we reach \f$K_{max}\f$, we'll try reheating instead of terminating,
  // allowing a fast annealing schedule to be used.
  if (state->number_of_state_changes >= ANNEALING_STEP_MAX - 1) {
    reheat(state);
  }

  carr2_u8 new_state = carr2_u8_init(9, 9);

  carr2_u8_copy(&new_state, *state->sudoku_puzzle_state);

  const double random_number_range_zero_to_one =
      ((double)random_uint32_t(state->random_number_generator_state) /
       (double)UINT32_MAX);

  // \f$s_{new} \leftarrow neighbour(s)\f$
  select_neighbouring_state(state, &new_state);

  const uint32_t cost_of_new_state = cost(new_state.data);

  const int32_t cost_difference =
      (cost_of_new_state - state->sudoku_puzzle_state_cost);

  const double acceptance_probability =
      exp((-1.0 * cost_difference) / state->temperature);

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
