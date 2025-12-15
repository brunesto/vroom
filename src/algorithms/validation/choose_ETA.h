#ifndef CHOOSE_ETA_H
#define CHOOSE_ETA_H

/*

This file is part of VROOM.

Copyright (c) 2015-2025, Julien Coupey.
All rights reserved (see LICENSE).

*/

#include "structures/vroom/input/input.h"
#include "structures/vroom/solution/route.h"

namespace vroom::validation {

/**
 * LLM: @brief Computes optimal arrival times for a route considering time windows.
 *
 * Uses linear programming to determine arrival times at each step that minimize
 * time window violations (both earliness and lateness). Balances trade-offs when
 * satisfying all time windows simultaneously is impossible.
 *
 * @param input Input problem instance with jobs and time window constraints.
 * @param vehicle_rank Rank of the vehicle for this route.
 * @param steps Ordered sequence of vehicle steps defining the route.
 * @return Route with computed arrival times and service times for each step.
 */
Route choose_ETA(const Input& input,
                 unsigned vehicle_rank,
                 const std::vector<VehicleStep>& steps);

} // namespace vroom::validation

#endif
