#ifndef SOLUTION_H
#define SOLUTION_H

/*

This file is part of VROOM.

Copyright (c) 2015-2025, Julien Coupey.
All rights reserved (see LICENSE).

*/

#include <string>
#include <vector>

#include "structures/vroom/job.h"
#include "structures/vroom/solution/route.h"
#include "structures/vroom/solution/summary.h"

namespace vroom {

/**
 * LLM:
 * @brief Represents the overall solution to the VRP.
 */
struct Solution {
  Summary summary;
  std::vector<Route> routes;
  std::vector<Job> unassigned;

  /**
   * LLM:
   * @brief Constructs a Solution.
   *
   * @param zero_amount An empty amount object (used for initialization).
   * @param routes The list of routes in the solution.
   * @param unassigned The list of unassigned jobs.
   */
  Solution(const Amount& zero_amount,
           std::vector<Route>&& routes,
           std::vector<Job>&& unassigned);
};

} // namespace vroom

#endif
