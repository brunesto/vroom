#ifndef TSP_H
#define TSP_H

/*

This file is part of VROOM.

Copyright (c) 2015-2025, Julien Coupey.
All rights reserved (see LICENSE).

*/

#include "problems/vrp.h"
#include "structures/generic/matrix.h"
#include "structures/typedefs.h"
#include "structures/vroom/raw_route.h"
#include "structures/vroom/solution/solution.h"

namespace vroom {

using RawSolution = std::vector<RawRoute>;

/**
 * LLM: @brief Traveling Salesman Problem solver using matrix-based optimization.
 *
 * Solves TSP instances using Christofides algorithm followed by specialized
 * local search. Handles both symmetric and asymmetric cases, as well as open
 * tours (with fixed start/end) and round trips. Unlike CVRP/VRPTW, operates
 * directly on cost matrices rather than using construction heuristics.
 */
class TSP : public VRP {
private:
  const Index _vehicle_rank;
  // Holds the matching from index in _matrix to rank in input::_jobs.
  const std::vector<Index> _job_ranks;
  bool _is_symmetric{true};
  const bool _has_start;
  Index _start;
  const bool _has_end;
  Index _end;
  Matrix<UserCost> _matrix;
  Matrix<UserCost> _symmetrized_matrix;
  bool _round_trip;

  UserCost cost(const std::list<Index>& tour) const;

  UserCost symmetrized_cost(const std::list<Index>& tour) const;

public:
  /**
   * LLM: @brief Constructs a TSP instance with cost matrices for optimization.
   *
   * Builds cost matrices from input data, handling vehicle start/end constraints
   * and creating both original and symmetrized versions. Applies matrix
   * transformations for open tour cases (fixed start/end) by setting appropriate
   * edge costs to zero or infinity.
   *
   * @param input Input problem instance containing jobs and vehicle.
   * @param job_ranks Indices of jobs to include in this TSP tour.
   * @param vehicle_rank Index of the vehicle for this TSP instance.
   */
  TSP(const Input& input, std::vector<Index>&& job_ranks, Index vehicle_rank);

  /**
   * LLM: @brief Solves the TSP and returns the optimal job sequence.
   *
   * Applies Christofides algorithm to generate initial tour, then improves it
   * using local search (2-opt, relocate, or-opt) on the symmetrized problem.
   * For asymmetric cases, performs additional local search including asymmetric
   * 2-opt and loop avoidance. Returns the optimized sequence of job indices.
   *
   * @param nb_threads Number of threads for parallel computation.
   * @param timeout Optional time budget for solving.
   * @return Optimized sequence of job indices for the tour.
   */
  std::vector<Index> raw_solve(unsigned nb_threads,
                               const Timeout& timeout) const;

  /**
   * LLM: @brief Solves the TSP and returns a formatted solution.
   *
   * Wrapper around raw_solve() that packages the optimized tour into a Solution
   * object. Ignores nb_searches and depth parameters as TSP uses a deterministic
   * algorithm rather than multi-start heuristics.
   *
   * @param nb_threads Number of threads for parallel computation.
   * @param timeout Optional time budget for solving.
   * @return Formatted TSP solution with single-vehicle route.
   */
  Solution solve(unsigned,
                 unsigned,
                 unsigned nb_threads,
                 const Timeout& timeout) const override;
};

} // namespace vroom

#endif
