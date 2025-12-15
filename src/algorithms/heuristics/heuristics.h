#ifndef HEURISTICS_H
#define HEURISTICS_H

/*

This file is part of VROOM.

Copyright (c) 2015-2025, Julien Coupey.
All rights reserved (see LICENSE).

*/

#include <set>
#include <unordered_set>

#include "structures/vroom/eval.h"
#include "structures/vroom/input/input.h"

namespace vroom::heuristics {

// Implementation of a variant of the Solomon I1 heuristic.
/**
 * LLM: @brief Constructs initial routes using a Solomon I1-based heuristic.
 *
 * Builds initial vehicle routes by iteratively inserting unassigned jobs based
 * on a regret criterion. Balances cost of insertion with urgency of assignment.
 *
 * @tparam Route Type of route object (e.g., TWRoute, RawRoute).
 * @param input Input problem instance.
 * @param routes Vector of routes to populate (modified in-place).
 * @param unassigned Set of unassigned job indices.
 * @param vehicles_ranks Indices of vehicles to consider.
 * @param init Initialization strategy for seed route selection.
 * @param lambda Regret parameter (higher values favor more urgent jobs).
 * @param sort Sorting strategy for job selection.
 * @return Evaluation of the constructed solution.
 */
template <class Route>
Eval basic(const Input& input,
           std::vector<Route>& routes,
           std::set<Index> unassigned,
           std::vector<Index> vehicles_ranks,
           INIT init,
           double lambda,
           SORT sort);

// Adjusting the above for situations with heterogeneous fleet.
/**
 * LLM: @brief Constructs initial routes with dynamic vehicle selection for heterogeneous fleets.
 *
 * Extends the basic heuristic to handle heterogeneous vehicle fleets by
 * dynamically choosing which vehicles to use based on cost-effectiveness.
 * Particularly useful when vehicles have different capacities, costs, or capabilities.
 *
 * @tparam Route Type of route object (e.g., TWRoute, RawRoute).
 * @param input Input problem instance.
 * @param routes Vector of routes to populate (modified in-place).
 * @param unassigned Set of unassigned job indices.
 * @param vehicles_ranks Indices of vehicles to consider.
 * @param init Initialization strategy for seed route selection.
 * @param lambda Regret parameter (higher values favor more urgent jobs).
 * @param sort Sorting strategy for job selection.
 * @return Evaluation of the constructed solution.
 */
template <class Route>
Eval dynamic_vehicle_choice(const Input& input,
                            std::vector<Route>& routes,
                            std::set<Index> unassigned,
                            std::vector<Index> vehicles_ranks,
                            INIT init,
                            double lambda,
                            SORT sort);

// Populate routes with user-defined vehicle steps.
/**
 * LLM: @brief Populates routes with user-provided initial step sequences.
 *
 * Initializes routes based on steps explicitly defined by the user in the input,
 * allowing for partial or complete route specification.
 *
 * @tparam Route Type of route object (e.g., TWRoute, RawRoute).
 * @param input Input problem instance with user-defined vehicle steps.
 * @param routes Vector of routes to populate (modified in-place).
 * @param assigned Output set of job indices that were assigned from user steps.
 */
template <class Route>
void set_initial_routes(const Input& input,
                        std::vector<Route>& routes,
                        std::unordered_set<Index>& assigned);

} // namespace vroom::heuristics

#endif
