// SPDX-License-Identifier: ISC

#include "interface.h"
#include <notcurses/notcurses.h>
#include <stc/cstr.h>
#include <stdlib.h>

static struct notcurses *notcurses;
static struct ncplane *standard_plane;
static struct ncplane *sudoku_board_plane;
static struct ncplane *sudoku_state_plane;
static nccell upper_left = NCCELL_TRIVIAL_INITIALIZER;
static nccell upper_right = NCCELL_TRIVIAL_INITIALIZER;
static nccell lower_left = NCCELL_TRIVIAL_INITIALIZER;
static nccell lower_right = NCCELL_TRIVIAL_INITIALIZER;
static nccell horizontal_line = NCCELL_TRIVIAL_INITIALIZER;
static nccell vertical_line = NCCELL_TRIVIAL_INITIALIZER;

#define BOARD_HEIGHT 19
#define BOARD_WIDTH 37

void blit_sudoku_grid(void);

void blit_sudoku_numbers(annealing_state *state);

void initialize_user_interface(void) {
  struct notcurses_options options = {0};
  notcurses = notcurses_init(&options, NULL);
  if (!notcurses || !notcurses_canopen_images(notcurses)) {
    exit(EXIT_FAILURE);
  }

  standard_plane = notcurses_stdplane(notcurses);

  unsigned xlen;
  unsigned ylen;
  ncplane_dim_yx(standard_plane, &ylen, &xlen);

  struct ncplane_options sudoku_board_plane_options = {
      .y = ylen / 2.0 - BOARD_HEIGHT / 2,
      .x = xlen / 2.0 - BOARD_WIDTH / 2,
      .rows = BOARD_HEIGHT,
      .cols = BOARD_WIDTH,
      .userptr = NULL,
      .name = NULL,
      .resizecb = NULL,
      .flags = 0,
      .margin_b = 0,
      .margin_r = 0};

  sudoku_board_plane =
      ncplane_create(standard_plane, &sudoku_board_plane_options);

  nccell sudoku_board_plane_base_cell = NCCELL_TRIVIAL_INITIALIZER;
  if (nccell_set_bg_rgb8(&sudoku_board_plane_base_cell, 0xff, 0xff, 0xff) |
      nccell_set_fg_rgb8(&sudoku_board_plane_base_cell, 0, 0, 0)) {
    exit(EXIT_FAILURE);
  }

  if (ncplane_set_base_cell(standard_plane, &sudoku_board_plane_base_cell) |
      ncplane_set_base_cell(sudoku_board_plane,
                            &sudoku_board_plane_base_cell)) {
    exit(EXIT_FAILURE);
  }

  if (nccells_load_box(sudoku_board_plane, 0, 0, &upper_left, &upper_right,
                       &lower_left, &lower_right, &horizontal_line,
                       &vertical_line, NCBOXHEAVY)) {
    exit(EXIT_FAILURE);
  }

  if (nccell_set_bg_rgb8(&upper_left, 0xff, 0xff, 0xff) |
      nccell_set_fg_rgb8(&upper_left, 0, 0, 0) |
      nccell_set_bg_rgb8(&upper_right, 0xff, 0xff, 0xff) |
      nccell_set_fg_rgb8(&upper_right, 0, 0, 0) |
      nccell_set_bg_rgb8(&lower_left, 0xff, 0xff, 0xff) |
      nccell_set_fg_rgb8(&lower_left, 0, 0, 0) |
      nccell_set_bg_rgb8(&lower_right, 0xff, 0xff, 0xff) |
      nccell_set_fg_rgb8(&lower_right, 0, 0, 0) |
      nccell_set_bg_rgb8(&horizontal_line, 0xff, 0xff, 0xff) |
      nccell_set_fg_rgb8(&horizontal_line, 0, 0, 0) |
      nccell_set_bg_rgb8(&vertical_line, 0xff, 0xff, 0xff) |
      nccell_set_fg_rgb8(&vertical_line, 0, 0, 0)) {
    exit(EXIT_FAILURE);
  }

  if (ncplane_box_sized(sudoku_board_plane, &upper_left, &upper_right,
                        &lower_left, &lower_right, &horizontal_line,
                        &vertical_line, sudoku_board_plane_options.rows,
                        sudoku_board_plane_options.cols, 0)) {
    exit(EXIT_FAILURE);
  }

  struct ncplane_options sudoku_state_plane_options = {
      .y = sudoku_board_plane_options.y + 1,
      .x = sudoku_board_plane_options.x + 1,
      .rows = BOARD_HEIGHT - 2,
      .cols = BOARD_WIDTH - 2,
      .userptr = NULL,
      .name = NULL,
      .resizecb = NULL,
      .flags = 0,
      .margin_b = 0,
      .margin_r = 0};

  sudoku_state_plane =
      ncplane_create(standard_plane, &sudoku_state_plane_options);

  ncplane_set_fg_rgb8(sudoku_state_plane, 0, 0, 0);
  ncplane_set_bg_rgb8(sudoku_state_plane, 0xff, 0xff, 0xff);

  if (ncplane_set_base_cell(sudoku_state_plane,
                            &sudoku_board_plane_base_cell)) {
    exit(EXIT_FAILURE);
  }
}

void wait_for_user_input(void) {
  ncinput notcurses_input;
  uint32_t id;

  while ((id = notcurses_get_blocking(notcurses, &notcurses_input)) !=
         (uint32_t)-1) {
    if (id == 0) {
      continue; // timeout waiting for input
    }

    return;
  }

  __builtin_unreachable();
}

void blit_sudoku_grid(void) {
  nccell line_cell = NCCELL_TRIVIAL_INITIALIZER;

  if (nccell_set_bg_rgb8(&line_cell, 0xff, 0xff, 0xff) |
      nccell_set_fg_rgb8(&line_cell, 0, 0, 0)) {
    exit(EXIT_FAILURE);
  }

  nccell_load(sudoku_state_plane, &line_cell, "─");
  uint8_t thin_hline_coordinates[6] = {1, 3, 7, 9, 13, 15};
  for (size_t i = 0; i < 6; i++) {
    ncplane_cursor_move_yx(sudoku_state_plane, thin_hline_coordinates[i], 0);
    ncplane_hline(sudoku_state_plane, &line_cell, BOARD_WIDTH);
  }

  nccell_load(sudoku_state_plane, &line_cell, "│");
  uint8_t thin_vline_coordinates[6] = {3, 7, 15, 19, 27, 31};
  for (size_t i = 0; i < 6; i++) {
    ncplane_cursor_move_yx(sudoku_state_plane, 0, thin_vline_coordinates[i]);
    ncplane_vline(sudoku_state_plane, &line_cell, BOARD_HEIGHT - 1);
  }

  nccell_load(sudoku_state_plane, &line_cell, "━");
  uint8_t thick_hline_coordinates[2] = {5, 11};
  for (size_t i = 0; i < 2; i++) {
    ncplane_cursor_move_yx(sudoku_state_plane, thick_hline_coordinates[i], 0);
    ncplane_hline(sudoku_state_plane, &line_cell, BOARD_WIDTH);
  }

  nccell_load(sudoku_state_plane, &line_cell, "┃");
  uint8_t thick_vline_coordinates[2] = {11, 23};
  for (size_t i = 0; i < 2; i++) {
    ncplane_cursor_move_yx(sudoku_state_plane, 0, thick_vline_coordinates[i]);
    ncplane_vline(sudoku_state_plane, &line_cell, BOARD_HEIGHT - 1);
  }

  for (size_t i = 0; i < 2; i++) {
    for (size_t j = 0; j < 2; j++) {
      ncplane_putstr_yx(sudoku_state_plane, thick_hline_coordinates[j],
                        thick_vline_coordinates[i], "╋");
    }
  }

  for (size_t i = 0; i < 6; i++) {
    for (size_t j = 0; j < 6; j++) {
      ncplane_putstr_yx(sudoku_state_plane, thin_hline_coordinates[j],
                        thin_vline_coordinates[i], "┼");
    }
  }

  for (size_t i = 0; i < 2; i++) {
    for (size_t j = 0; j < 6; j++) {
      ncplane_putstr_yx(sudoku_state_plane, thin_hline_coordinates[j],
                        thick_vline_coordinates[i], "╂");
      ncplane_putstr_yx(sudoku_state_plane, thick_hline_coordinates[i],
                        thin_vline_coordinates[j], "┿");
    }
  }

  for (size_t i = 0; i < 6; i++) {
    ncplane_putstr_yx(sudoku_board_plane, 0, thin_vline_coordinates[i] + 1,
                      "┯");
    ncplane_putstr_yx(sudoku_board_plane, BOARD_HEIGHT - 1,
                      thin_vline_coordinates[i] + 1, "┷");
    ncplane_putstr_yx(sudoku_board_plane, thin_hline_coordinates[i] + 1, 0,
                      "┠");
    ncplane_putstr_yx(sudoku_board_plane, thin_hline_coordinates[i] + 1,
                      BOARD_WIDTH - 1, "┨");
  }

  for (size_t i = 0; i < 2; i++) {
    ncplane_putstr_yx(sudoku_board_plane, 0, thick_vline_coordinates[i] + 1,
                      "┳");
    ncplane_putstr_yx(sudoku_board_plane, BOARD_HEIGHT - 1,
                      thick_vline_coordinates[i] + 1, "┻");
    ncplane_putstr_yx(sudoku_board_plane, thick_hline_coordinates[i] + 1, 0,
                      "┣");
    ncplane_putstr_yx(sudoku_board_plane, thick_hline_coordinates[i] + 1,
                      BOARD_WIDTH - 1, "┫");
  }

  nccell_release(sudoku_state_plane, &line_cell);
}

void blit_sudoku_numbers(annealing_state *state) {
  uint32_t horizontal_offset = 1;
  uint32_t vertical_offset = 0;
  for (size_t y = 0; y < 9; y++) {
    for (size_t x = 0; x < 9; x++) {
      ncplane_set_fg_rgb8(sudoku_state_plane, 115, 147, 179);
      ncplane_set_bg_rgb8(sudoku_state_plane, 0xff, 0xff, 0xff);

      if (state->given_puzzle_positions->data[y][x] == 0) {
        ncplane_set_fg_rgb8(sudoku_state_plane, 46, 139, 87);
        ncplane_set_bg_rgb8(sudoku_state_plane, 0xff, 0xff, 0xff);
      }

      cstr number_at_yx =
          cstr_from_fmt("%d", state->sudoku_puzzle_state->data[y][x]);
      ncplane_putstr_yx(sudoku_state_plane, y + vertical_offset,
                        x + horizontal_offset, cstr_str(&number_at_yx));
      c_drop(cstr, &number_at_yx);

      if (state->given_puzzle_positions->data[y][x] == 0) {
        ncplane_set_fg_rgb8(sudoku_state_plane, 0, 0, 0);
        ncplane_set_bg_rgb8(sudoku_state_plane, 0xff, 0xff, 0xff);
      }

      horizontal_offset++;
      horizontal_offset++;
      horizontal_offset++;
    }
    horizontal_offset = 1;
    vertical_offset++;
  }

  unsigned xlen;
  unsigned ylen;
  ncplane_dim_yx(standard_plane, &ylen, &xlen);

  cstr temperature = cstr_from_fmt("%.4f°", state->temperature);

  ncplane_putstr_yx(standard_plane, (ylen / 2.0) + (BOARD_HEIGHT / 2) + 2,
                    (xlen / 2.0) - (BOARD_WIDTH / 2) + 2,
                     cstr_str(&temperature));
  c_drop(cstr, &temperature);

  cstr cost = cstr_from_fmt("$%04d", state->sudoku_puzzle_state_cost);

  ncplane_putstr_yx(standard_plane, (ylen / 2.0) + (BOARD_HEIGHT / 2) + 3,
                    (xlen / 2.0) - (BOARD_WIDTH / 2) + 2,
                    cstr_str(&cost));
  c_drop(cstr, &cost);
}

void update_user_interface(annealing_state *state) {
  blit_sudoku_grid();
  blit_sudoku_numbers(state);
  notcurses_render(notcurses);
}

void deinitialize_user_interface(void) {
  notcurses_render(notcurses);

  ncplane_destroy(sudoku_state_plane);

  nccell_release(sudoku_board_plane, &upper_left);
  nccell_release(sudoku_board_plane, &upper_right);
  nccell_release(sudoku_board_plane, &lower_left);
  nccell_release(sudoku_board_plane, &lower_right);
  nccell_release(sudoku_board_plane, &horizontal_line);
  nccell_release(sudoku_board_plane, &vertical_line);

  ncplane_destroy(sudoku_board_plane);
  notcurses_stop(notcurses);
}
