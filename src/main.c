// This program solves sudoku puzzles using a global optimization technique
// called Simulated Annealing, a type of Markov chain Monte Carlo algorithm.

// The general outline of the algorithm is starting from state \f$s_0\f$, and
// continuing until at most \f$k_{max}\f$ state changes, change to neighbouring
// states based on acceptance probabilities and an annealing schedule. The
// probability of transitioning to a neighbouring state is specified by an
// acceptance probability function \f$P({cost}, {cost}_{new}, T)\f$ that tends
// to \f$0\f$ when the annealing temperature \f$T\f$ tends to \f$0\f$ if
// \f${cost}_{new} > {cost}\f$. Meaning as \f$T\f$ tends to \f$0\f$ accepting a
// state with a higher cost stops taking place, allowing the system to
// increasingly favor state transitions towards a local minimum cost over time
// based on the annealing schedule (the rate that annealing temperature
// decreases).
//
//  Let \f$s = s_0\f$
//  For \f$k = 0\f$ through, excluding, \f$k_{max}\f$:
//    \f$T \leftarrow temperature(1-(k+1)/k_{max})\f$
//    Pick a random neighbour, \f$s_{new} \leftarrow neighbour(s)\f$
//    If \f$P(cost(s),cost(s_{new}), T) \geq random(0,1)\f$:
//      \f$s \leftarrow s_{new}\f$
//  Return \f$s\f$
//
// Similar to annealing processes in metallurgy if a desired result is not
// achieved it is easy to try again by choosing a new initial state and
// increasing the temperature.

#include "sys/random.h"
#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

#include "annealing.h"
#include "config.h"
#include "interface.h"
#include "puzzle.h"

int main(int argc, char **argv) {
  if (argc != 10) {
    printf("Usage: " PROGRAM_NAME
           " 000000000 000000000 000000000 000000000 000000000 "
           "000000000 000000000 000000000 000000000\n");
    return EXIT_FAILURE;
  }

  initialize_user_interface();

  // Storage for the 9 x 9 puzzle state
  carr2_u8 given_puzzle_positions = carr2_u8_with_values(9, 9, 1);
  carr2_u8 n_by_n = carr2_u8_with_values(9, 9, 0);

  // Load initial puzzle state from ARGV
  for (size_t i = 0; i < 9; i++) {
    for (size_t j = 0; j < 9; j++) {
      n_by_n.data[i][j] = argv[i + 1][j] - '0';
      given_puzzle_positions.data[i][j] = (n_by_n.data[i][j] != 0);
    }
  }

  // Storage for a copy of the unsolved state, used to reset
  // the annealing process.
  carr2_u8 initial_puzzle_positions = carr2_u8_init(9, 9);

  annealing_state puzzle_state = {
      .annealing = true,
      .temperature = 0,
      .initial_puzzle_state = &initial_puzzle_positions,
      .sudoku_puzzle_state = &n_by_n,
      .given_puzzle_positions = &given_puzzle_positions,
      .number_of_state_changes = 0,
      .sudoku_puzzle_state_cost = 9999,
      .random_number_generator_state = {0}};

  carr2_u8_copy(&initial_puzzle_positions, n_by_n);

  // Seed the random number generator with random data generated
  // by the system.
  if (getrandom(puzzle_state.random_number_generator_state,
                sizeof(puzzle_state.random_number_generator_state), 0) < 1) {
    return EXIT_FAILURE;
  }

  // Display the unsolved puzzle
  update_user_interface(&puzzle_state);
  wait_for_user_input();

  // Fill each 3x3 region of numbers randomly while maintaining the invariant
  // that each such region cannot contain duplicate numbers. This invariant will
  // be maintained when producing new puzzle states by swapping two numbers in a
  // region.
  fill_puzzle_regions(&puzzle_state);

  puzzle_state.sudoku_puzzle_state_cost =
      cost(puzzle_state.sudoku_puzzle_state->data);

  puzzle_state.temperature = estimate_initial_temperature(&puzzle_state);

  clock_t start = clock();
  clock_t current = clock();

  while (puzzle_state.annealing) {
    update_annealing_state(&puzzle_state);

    current = clock();
    if (((double)(current - start)) / CLOCKS_PER_SEC > 1.0) {
      update_user_interface(&puzzle_state);
      start = current;
    }
  }

  // Avoid a sudden update at the end by waiting until a full second has
  // elapsed.
  struct timespec part_of_a_second;
  part_of_a_second.tv_sec = 0;
  part_of_a_second.tv_nsec =
      1000000000 -
      (1000000000 * ((double)(current - start) / CLOCKS_PER_SEC));

  int was_interrupted;
  do {
    was_interrupted = nanosleep(&part_of_a_second, &part_of_a_second);
  } while (was_interrupted && errno == EINTR);

  // Display the solved puzzle
  update_user_interface(&puzzle_state);
  wait_for_user_input();

  deinitialize_user_interface();

  carr2_u8_drop(&initial_puzzle_positions);
  carr2_u8_drop(&n_by_n);
  carr2_u8_drop(&given_puzzle_positions);

  return EXIT_SUCCESS;
}
