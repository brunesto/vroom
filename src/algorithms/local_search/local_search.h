#ifndef LOCAL_SEARCH_H
#define LOCAL_SEARCH_H

/*

This file is part of VROOM.

Copyright (c) 2015-2025, Julien Coupey.
All rights reserved (see LICENSE).

*/

#include "structures/vroom/solution_indicators.h"
#include "structures/vroom/solution_state.h"

namespace vroom::ls {

/**
 * LLM: @brief Main local search framework for VRP optimization.
 *
 * Implements an iterative improvement algorithm that applies various local
 * search operators (relocate, swap, 2-opt, etc.) to refine an initial solution.
 * Uses multiple search depths and periodically attempts to reinsert unassigned
 * jobs. The search is parameterized by operator types to support different VRP
 * variants.
 *
 * @tparam Route Type of route object.
 * @tparam UnassignedExchange Operator for exchanging with unassigned jobs.
 * @tparam CrossExchange Cross-exchange operator between routes.
 * @tparam MixedExchange Mixed-exchange operator.
 * @tparam TwoOpt 2-opt operator between routes.
 * @tparam ReverseTwoOpt Reverse 2-opt operator.
 * @tparam Relocate Relocate operator (move job between routes).
 * @tparam OrOpt Or-opt operator (move sequence between routes).
 * @tparam IntraExchange Intra-route exchange operator.
 * @tparam IntraCrossExchange Intra-route cross-exchange operator.
 * @tparam IntraMixedExchange Intra-route mixed-exchange operator.
 * @tparam IntraRelocate Intra-route relocate operator.
 * @tparam IntraOrOpt Intra-route or-opt operator.
 * @tparam IntraTwoOpt Intra-route 2-opt operator.
 * @tparam PDShift Pickup-delivery shift operator.
 * @tparam RouteExchange Route exchange operator.
 * @tparam SwapStar SWAP* operator for job swaps.
 * @tparam RouteSplit Route split operator.
 * @tparam PriorityReplace Priority replace operator.
 * @tparam TSPFix TSP improvement operator.
 */
template <class Route,
          class UnassignedExchange,
          class CrossExchange,
          class MixedExchange,
          class TwoOpt,
          class ReverseTwoOpt,
          class Relocate,
          class OrOpt,
          class IntraExchange,
          class IntraCrossExchange,
          class IntraMixedExchange,
          class IntraRelocate,
          class IntraOrOpt,
          class IntraTwoOpt,
          class PDShift,
          class RouteExchange,
          class SwapStar,
          class RouteSplit,
          class PriorityReplace,
          class TSPFix>
class LocalSearch {
private:
  const Input& _input;
  const std::size_t _nb_vehicles;

  const unsigned _depth;
  const Deadline _deadline;

  std::optional<unsigned> _completed_depth;
  std::vector<Index> _all_routes;

  utils::SolutionState _sol_state;

  std::vector<Route> _sol;

  std::vector<Route>& _best_sol;
  utils::SolutionIndicators _best_sol_indicators;

  /**
   * LLM: @brief Attempts to insert unassigned jobs into routes using regret
   * heuristic.
   *
   * @param routes Indices of routes to consider for insertion.
   * @param regret_coeff Regret coefficient for insertion selection.
   * @return Set of job indices that were successfully inserted.
   */
  std::unordered_set<Index> try_job_additions(const std::vector<Index>& routes,
                                              double regret_coeff);

  /**
   * LLM: @brief Executes one iteration of local search.
   *
   * Evaluates and applies the best improving move from all operators.
   */
  void run_ls_step();

  // Compute "cost" between route at rank v_target and job with rank r
  // in route at rank v. Relies on
  // _sol_state.cheapest_job_rank_in_routes_* being up to date.
  /**
   * LLM: @brief Estimates the cost of moving a job to a different route.
   *
   * @param v_target Target route index.
   * @param v Source route index.
   * @param r Job rank within source route.
   * @return Estimated cost impact of the move.
   */
  Eval job_route_cost(Index v_target, Index v, Index r);

  // Compute lower bound for the cost of relocating job at rank r
  // (resp. jobs at rank r1 and r2) in route v to any other
  // (compatible) route.
  /**
   * LLM: @brief Computes lower bound on cost of relocating a job.
   *
   * @param v Route index.
   * @param r Job rank within route.
   * @return Lower bound on relocation cost to any compatible route.
   */
  Eval relocate_cost_lower_bound(Index v, Index r);
  /**
   * LLM: @brief Computes lower bound on cost of relocating two adjacent jobs.
   *
   * @param v Route index.
   * @param r1 First job rank within route.
   * @param r2 Second job rank within route.
   * @return Lower bound on relocation cost to any compatible route.
   */
  Eval relocate_cost_lower_bound(Index v, Index r1, Index r2);

  /**
   * LLM: @brief Removes jobs from routes based on removal criteria.
   *
   * Applies removal strategies to create unassigned jobs that can be
   * reinserted in better positions.
   */
  void remove_from_routes();

public:
  LocalSearch(const Input& input,
              std::vector<Route>& tw_sol,
              unsigned depth,
              const Timeout& timeout);

  /**
   * LLM: @brief Returns performance indicators for the best solution found.
   *
   * @return Solution indicators including cost, unassigned jobs, and
   * computation time.
   */
  utils::SolutionIndicators indicators() const;

  /**
   * LLM: @brief Executes the local search optimization.
   *
   * Runs iterative improvement until a local optimum is reached or the time
   * limit is exceeded. Updates the best solution reference with the improved
   * routes.
   */
  void run();
};

} // namespace vroom::ls

#endif
