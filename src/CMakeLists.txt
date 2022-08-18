add_library(containers STATIC containers.c)

add_executable(annealing-sudoku-solver main.c annealing.c interface.c puzzle.c rng.c)

set_property(TARGET annealing-sudoku-solver PROPERTY C_STANDARD 23)

set_target_properties(annealing-sudoku-solver PROPERTIES OUTPUT_NAME "${TARGET_OUTPUT_NAME}")

target_include_directories(annealing-sudoku-solver PUBLIC "${PROJECT_BINARY_DIR}")
target_include_directories(containers PUBLIC "${PROJECT_BINARY_DIR}")

target_link_libraries(annealing-sudoku-solver stc::stc notcurses notcurses-core containers m)
target_link_libraries(containers stc::stc)

target_compile_definitions(annealing-sudoku-solver PRIVATE STC_HEADER)
