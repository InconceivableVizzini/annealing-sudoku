add_executable(annealing-sudoku-solver main.c annealing.c interface.c)

set_property(TARGET annealing-sudoku-solver PROPERTY C_STANDARD 23)

set_target_properties(annealing-sudoku-solver PROPERTIES OUTPUT_NAME "${TARGET_OUTPUT_NAME}")
target_include_directories(annealing-sudoku-solver PUBLIC "${PROJECT_BINARY_DIR}")
target_link_libraries(annealing-sudoku-solver stc::stc notcurses notcurses-core)
