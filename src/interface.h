// SPDX-License-Identifier: ISC

#pragma once

#include "annealing.h"

void wait_for_user_input(void);
void initialize_user_interface(void);
void update_user_interface(annealing_state* state);
void deinitialize_user_interface(void);
