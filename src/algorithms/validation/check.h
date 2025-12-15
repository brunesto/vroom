#ifndef CHECK_H
#define CHECK_H

/*

This file is part of VROOM.

Copyright (c) 2015-2025, Julien Coupey.
All rights reserved (see LICENSE).

*/

#include <unordered_map>

#include "structures/vroom/input/input.h"
#include "structures/vroom/solution/solution.h"

namespace vroom::validation {

/**
 * LLM: @brief Validates user-provided vehicle steps and computes estimated times of arrival.
 *
 * Checks that all user-defined vehicle steps are valid (no duplicate jobs,
 * compatible with vehicle constraints, etc.) and computes the optimal arrival
 * times at each step considering time windows and travel times. Processes
 * routes in parallel using multiple threads.
 *
 * @param input Input problem instance with vehicles and jobs.
 * @param nb_thread Number of threads to use for parallel processing.
 * @param route_rank_to_v_rank Output mapping from route ranks to vehicle ranks.
 * @return Solution with validated routes and computed ETAs.
 * @throws InputException if validation fails (invalid steps, constraint
 * violations, etc.).
 */
Solution
check_and_set_ETA(const Input& input,
                  unsigned nb_thread,
                  std::unordered_map<Index, Index>& route_rank_to_v_rank);

} // namespace vroom::validation

#endif
