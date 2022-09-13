#include "puzzle.h"
#include "rng.h"

void fill_region(annealing_state *annealing_state, size_t region) {
  clist_u8 list_of_available_numbers = clist_u8_init();
  clist_u8 list_of_cell_x = clist_u8_init();
  clist_u8 list_of_cell_y = clist_u8_init();

  c_apply(n, clist_u8_push_back(&list_of_available_numbers, n), uint8_t,
          {1, 2, 3, 4, 5, 6, 7, 8, 9});

  // Randomly swap elements in the list of available numbers.
  for (size_t i = 0; i < 8; ++i) {
    size_t index =
        random_uint32_t(annealing_state->random_number_generator_state) %
        (9 - i);

    clist_u8_iter current_number_iter =
        clist_u8_begin(&list_of_available_numbers);
    current_number_iter = clist_u8_advance(current_number_iter, i);

    const uint8_t current_number_value = *current_number_iter.ref;

    clist_u8_iter some_number_iter = clist_u8_begin(&list_of_available_numbers);
    some_number_iter = clist_u8_advance(some_number_iter, index);

    *current_number_iter.ref = *some_number_iter.ref;
    *some_number_iter.ref = current_number_value;
  }

  // Create a list of available numbers and a list of positions that need to be
  // filled with numbers.
  for (size_t row = 0; row < 3; row++) {
    for (size_t column = 0; column < 3; column++) {
      const size_t cell_x_index = ((region / 3) * 3) + row;
      const size_t cell_y_index = ((region % 3) * 3) + column;
      const uint8_t cell_data = annealing_state->sudoku_puzzle_state
                                    ->data[cell_x_index][cell_y_index];
      if (cell_data > 0) {
        clist_u8_remove(&list_of_available_numbers, cell_data);
      } else {
        clist_u8_push_back(&list_of_cell_x, cell_x_index);
        clist_u8_push_back(&list_of_cell_y, cell_y_index);
      }
    }
  }

  // Fill positions with numbers from the list of available numbers.
  clist_u8_iter cell_x_iter = clist_u8_begin(&list_of_cell_x),
                cell_y_iter = clist_u8_begin(&list_of_cell_y),
                cell_x_end_iter = clist_u8_end(&list_of_cell_x);

  while (cell_x_iter.ref != cell_x_end_iter.ref) {
    const size_t cell_x_index = *cell_x_iter.ref;
    const size_t cell_y_index = *cell_y_iter.ref;
    const uint8_t *new_cell_value = clist_u8_front(&list_of_available_numbers);

    annealing_state->sudoku_puzzle_state->data[cell_x_index][cell_y_index] =
        *new_cell_value;

    clist_u8_pop_front(&list_of_available_numbers);

    clist_u8_next(&cell_x_iter);
    clist_u8_next(&cell_y_iter);
  }

  clist_u8_drop(&list_of_cell_y);
  clist_u8_drop(&list_of_cell_x);
  clist_u8_drop(&list_of_available_numbers);
}

void fill_puzzle_regions(annealing_state *puzzle_state) {
  for (size_t region = 0; region < 9; region++) {
    fill_region(puzzle_state, region);
  }
}
