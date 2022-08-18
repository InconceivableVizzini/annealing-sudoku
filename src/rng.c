#include "rng.h"

// xoshiro128++ 1.0 devised by David Blackman and Sebastiano Vigna
uint32_t random_uint32_t(uint32_t *random_number_generator_state) {
  const uint32_t mixed_up_bits =
      random_number_generator_state[0] + random_number_generator_state[3];
  const uint32_t random_number =
      ((mixed_up_bits << 7) | (mixed_up_bits >> 25)) +
      random_number_generator_state[0];
  const uint32_t bigified_number = random_number_generator_state[1] << 9;
  random_number_generator_state[2] ^= random_number_generator_state[0];
  random_number_generator_state[3] ^= random_number_generator_state[1];
  random_number_generator_state[1] ^= random_number_generator_state[2];
  random_number_generator_state[0] ^= random_number_generator_state[3];

  random_number_generator_state[2] ^= bigified_number;
  random_number_generator_state[3] = ((random_number_generator_state[3] << 11) |
                                      (random_number_generator_state[3] >> 21));

  return random_number;
}
