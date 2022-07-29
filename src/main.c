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

#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

#define i_val uint8_t
#define i_tag u8
#include <stc/carr2.h>

#include <stc/cstr.h>

#include "config.h"
#include "annealing.h"
#include "interface.h"

int main(int argc, char **argv) {
  if (argc != 10) {
    printf("Usage: " PROGRAM_NAME
           " 000000000 000000000 000000000 000000000 000000000 "
           "000000000 000000000 000000000 000000000\n");
    return EXIT_FAILURE;
  }

  initialize_user_interface();

  c_autovar(carr2_u8 given_positions = carr2_u8_with_values(9, 9, 1),
            carr2_u8_drop(&given_positions))
  c_autovar(carr2_u8 n_by_n = carr2_u8_with_values(9, 9, 0),
                carr2_u8_drop(&n_by_n))
  {
    for (size_t i = 0; i < 9; i++) {
      for (size_t j = 0; j < 9; j++) {
        n_by_n.data[i][j] = argv[i + 1][j] - '0';
        given_positions.data[i][j] = (n_by_n.data[i][j] != 0);
      }
    }

    annealing_state puzzle_state = {
        .annealing = true,
        .temperature =
            estimate_initial_temperature(n_by_n.data, given_positions.data),
        .sudoku_puzzle_state = n_by_n.data,
        .given_puzzle_positions = given_positions.data};

    
    update_user_interface(&puzzle_state);

    // Display the unsolved puzzle
    wait_for_user_input();

    clock_t start, current;
    start = clock();
    while (puzzle_state.annealing) {
      update_annealing_state(&puzzle_state);

      current = clock();
      if (((double)(current - start)) / CLOCKS_PER_SEC > 1.0) {
        update_user_interface(&puzzle_state);
        start = current;
      }
    }
  }

  // Display the solved puzzle
  wait_for_user_input();

  deinitialize_user_interface();

  return EXIT_SUCCESS;
}
