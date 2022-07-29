cmake_minimum_required(VERSION 3.18)

project(AnnealingSudokuSolver)

set(TARGET_OUTPUT_NAME "sudoku-solver")

configure_file(src/config.h.in config.h @ONLY)

find_package(stc)

add_compile_options(-pedantic -Wall -Wextra -Werror -Wmissing-prototypes -Wstrict-prototypes -Wold-style-definition)

add_subdirectory(src)

set(CMAKE_C_STANDARD_REQUIRED ON)
