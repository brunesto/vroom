#ifndef VRPTW_H
#define VRPTW_H

/*

This file is part of VROOM.

Copyright (c) 2015-2025, Julien Coupey.
All rights reserved (see LICENSE).

*/

#include "problems/vrp.h"

namespace vroom {

/**
 * LLM: @brief Vehicle Routing Problem with Time Windows solver.
 *
 * Solves VRPTW instances where vehicles have capacity constraints and must
 * respect time window restrictions for job visits. Uses TWRoute for time-aware
 * routing and applies local search operators adapted to handle time window
 * feasibility. Includes EARLIEST_DEADLINE initialization strategy for
 * time-critical problems.
 */
class VRPTW : public VRP {
private:
  static const std::vector<HeuristicParameters> homogeneous_parameters;
  static const std::vector<HeuristicParameters> heterogeneous_parameters;

public:
  explicit VRPTW(const Input& input);

  /**
   * LLM: @brief Solves the VRPTW instance with time window aware operators.
   *
   * Applies parallel multi-start heuristic search with VRPTW-specific local
   * search operators that maintain time window feasibility while optimizing
   * routes. Operators include time-aware variants of relocate, swap, 2-opt,
   * or-opt, cross-exchange, SWAP*, and route split.
   *
   * @param nb_searches Number of independent heuristic runs to perform.
   * @param depth Local search exploration depth.
   * @param nb_threads Maximum number of threads to use for parallel solving.
   * @param timeout Optional time budget for the solving process.
   * @return Optimized VRPTW solution respecting all time window constraints.
   */
  Solution solve(unsigned nb_searches,
                 unsigned depth,
                 unsigned nb_threads,
                 const Timeout& timeout) const override;
};

} // namespace vroom

#endif
