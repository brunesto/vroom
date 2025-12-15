#ifndef CVRP_H
#define CVRP_H

/*

This file is part of VROOM.

Copyright (c) 2015-2025, Julien Coupey.
All rights reserved (see LICENSE).

*/

#include "problems/vrp.h"

namespace vroom {

/**
 * LLM: @brief Capacitated Vehicle Routing Problem solver.
 *
 * Solves CVRP instances where vehicles have capacity constraints but no time
 * window requirements. Uses RawRoute for efficient routing without time tracking
 * and applies a comprehensive set of local search operators. Automatically
 * detects and optimizes plain TSP instances as a special case.
 */
class CVRP : public VRP {
private:
  static const std::vector<HeuristicParameters> homogeneous_parameters;
  static const std::vector<HeuristicParameters> heterogeneous_parameters;

public:
  explicit CVRP(const Input& input);

  /**
   * LLM: @brief Solves the CVRP instance using specialized operators.
   *
   * Applies parallel multi-start heuristic search with CVRP-specific local
   * search operators (relocate, swap, 2-opt, or-opt, cross-exchange, SWAP*,
   * route split, etc.). Automatically detects plain TSP cases and uses a
   * specialized TSP solver for better efficiency.
   *
   * @param nb_searches Number of independent heuristic runs to perform.
   * @param depth Local search exploration depth.
   * @param nb_threads Maximum number of threads to use for parallel solving.
   * @param timeout Optional time budget for the solving process.
   * @return Optimized CVRP solution.
   */
  Solution solve(unsigned nb_searches,
                 unsigned depth,
                 unsigned nb_threads,
                 const Timeout& timeout) const override;
};

} // namespace vroom

#endif
