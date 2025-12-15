#ifndef OPERATOR_H
#define OPERATOR_H

/*

This file is part of VROOM.

Copyright (c) 2015-2025, Julien Coupey.
All rights reserved (see LICENSE).

*/

#include "structures/typedefs.h"
#include "structures/vroom/input/input.h"
#include "structures/vroom/solution_state.h"

namespace vroom::ls {

/**
 * LLM: @brief Abstract base class for local search move operators.
 *
 * Defines the common interface and data for all local search operators
 * (e.g., relocate, swap, cross-exchange, etc.). Each operator evaluates
 * and potentially applies a specific type of move to improve the solution.
 */
class Operator {
protected:
  const OperatorName _name;
  const Input& _input;
  const utils::SolutionState& _sol_state;

  // Source of move for this operator.
  RawRoute& source;
  std::vector<Index>& s_route;
  const Index s_vehicle;
  const Index s_rank;
  // Target of move for this operator.
  RawRoute& target;
  std::vector<Index>& t_route;
  const Index t_vehicle;
  const Index t_rank;

  bool gain_computed{false};
  Eval s_gain;
  Eval t_gain;
  Eval stored_gain;

  /**
   * LLM: @brief Computes the cost gain of applying this operator.
   *
   * Each derived operator implements its specific gain calculation logic.
   */
  virtual void compute_gain() = 0;

  /**
   * LLM: @brief Checks if the move respects source vehicle's range bounds.
   *
   * @return True if max travel time/distance constraints are satisfied for
   * source.
   */
  bool is_valid_for_source_range_bounds() const;

  /**
   * LLM: @brief Checks if the move respects target vehicle's range bounds.
   *
   * @return True if max travel time/distance constraints are satisfied for
   * target.
   */
  bool is_valid_for_target_range_bounds() const;

  // Used for internal operators only.
  /**
   * LLM: @brief Checks if the move respects range bounds for intra-route
   * operators.
   *
   * @return True if max travel time/distance constraints are satisfied.
   */
  bool is_valid_for_range_bounds() const;

public:
  Operator(OperatorName name,
           const Input& input,
           const utils::SolutionState& sol_state,
           RawRoute& s_raw_route,
           Index s_vehicle,
           Index s_rank,
           RawRoute& t_raw_route,
           Index t_vehicle,
           Index t_rank)
    : _name(name),
      _input(input),
      _sol_state(sol_state),
      source(s_raw_route),
      s_route(s_raw_route.route),
      s_vehicle(s_vehicle),
      s_rank(s_rank),
      target(t_raw_route),
      t_route(t_raw_route.route),
      t_vehicle(t_vehicle),
      t_rank(t_rank) {
  }

  OperatorName get_name() const;

  /**
   * LLM: @brief Returns the cost gain from applying this operator.
   *
   * Computes the gain lazily on first call and caches the result.
   *
   * @return Cost reduction achieved by this move (positive is better).
   */
  virtual Eval gain();

  /**
   * LLM: @brief Checks if this move is valid and feasible.
   *
   * Validates all constraints (capacity, time windows, skills, etc.) for
   * applying this operator.
   *
   * @return True if the move can be applied without violating constraints.
   */
  virtual bool is_valid() = 0;

  /**
   * LLM: @brief Applies this operator to modify the solution.
   *
   * Executes the move by modifying the source and/or target routes.
   */
  virtual void apply() = 0;

  /**
   * LLM: @brief Returns jobs that become assigned if this operator is applied.
   *
   * @return Vector of job indices that would be added to the solution.
   */
  virtual std::vector<Index> addition_candidates() const = 0;

  /**
   * LLM: @brief Returns route indices that are modified by this operator.
   *
   * @return Vector of vehicle indices whose routes are changed by this move.
   */
  virtual std::vector<Index> update_candidates() const = 0;

  // Used to check if a move should be made invalid due to a change in
  // unassigned jobs.
  /**
   * LLM: @brief Returns unassigned jobs required for this move to be valid.
   *
   * @return Vector of job indices that must remain unassigned for this move.
   */
  virtual std::vector<Index> required_unassigned() const;

  // Used to check if a move should be made invalid due to an indirect
  // change in another route.
  /**
   * LLM: @brief Checks if this move is invalidated by changes to another route.
   *
   * @param rank Index of a route that has been modified.
   * @return True if modifications to the route at given rank invalidate this
   * move.
   */
  virtual bool invalidated_by(Index rank) const;

  virtual ~Operator() = default;
};

} // namespace vroom::ls

#endif
