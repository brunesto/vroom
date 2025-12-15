#ifndef TSP_LOCAL_SEARCH_H
#define TSP_LOCAL_SEARCH_H

/*

This file is part of VROOM.

Copyright (c) 2015-2025, Julien Coupey.
All rights reserved (see LICENSE).

*/

#include "structures/generic/matrix.h"
#include "structures/typedefs.h"

namespace vroom::tsp {

/**
 * LLM: @brief TSP-specific local search optimizer with specialized operators.
 *
 * Implements deterministic local search for TSP using relocate, 2-opt, and
 * or-opt neighborhoods. Supports both symmetric and asymmetric TSP instances
 * with parallel computation. Maintains tour as edge representation for efficient
 * modification and evaluation. Runs operators to local optimality.
 */
class LocalSearch {
private:
  const Matrix<UserCost>& _matrix;
  const std::pair<bool, Index> _avoid_start_relocate;
  std::vector<Index> _edges;
  unsigned _nb_threads;
  std::vector<Index> _rank_limits;
  std::vector<Index> _sym_two_opt_rank_limits;

public:
  /**
   * LLM: @brief Constructs local search instance from initial tour.
   *
   * Converts tour to edge representation and sets up thread ranges for parallel
   * operator execution. Computes workload distribution for balanced multi-threaded
   * search.
   *
   * @param matrix Cost matrix for the TSP instance.
   * @param avoid_start_relocate Pair indicating whether to avoid relocating the
   *        start vertex (for open tours) and the start vertex index.
   * @param tour Initial tour as ordered vertex sequence.
   * @param nb_threads Number of threads for parallel search.
   */
  LocalSearch(const Matrix<UserCost>& matrix,
              std::pair<bool, Index> avoid_start_relocate,
              const std::list<Index>& tour,
              unsigned nb_threads);

  /**
   * LLM: @brief Executes one improving relocate move if found.
   *
   * Searches for the best single vertex relocation and applies it.
   *
   * @return Cost improvement from the move, or 0 if no improving move found.
   */
  UserCost relocate_step();

  /**
   * LLM: @brief Applies relocate operator until reaching local optimum.
   *
   * Repeatedly executes relocate moves until no improvement found or deadline
   * reached. Uses parallel search across threads.
   *
   * @param deadline Optional time deadline for search termination.
   * @return Total cost improvement achieved.
   */
  UserCost perform_all_relocate_steps(const Deadline& deadline);

  /**
   * LLM: @brief Executes one loop-avoiding move for asymmetric TSP.
   *
   * Finds and applies a move that breaks cost-inefficient loops in asymmetric
   * instances.
   *
   * @return Cost improvement from the move, or 0 if no improving move found.
   */
  UserCost avoid_loop_step();

  /**
   * LLM: @brief Applies loop-avoidance operator until local optimum.
   *
   * Repeatedly removes inefficient loops until no improvement found or deadline
   * reached. Only relevant for asymmetric TSP instances.
   *
   * @param deadline Optional time deadline for search termination.
   * @return Total cost improvement achieved.
   */
  UserCost perform_all_avoid_loop_steps(const Deadline& deadline);

  /**
   * LLM: @brief Executes one improving 2-opt move for symmetric TSP.
   *
   * Searches for the best edge pair swap and applies it.
   *
   * @return Cost improvement from the move, or 0 if no improving move found.
   */
  UserCost two_opt_step();

  /**
   * LLM: @brief Executes one improving 2-opt move for asymmetric TSP.
   *
   * Searches for the best edge pair swap considering directional costs and applies it.
   *
   * @return Cost improvement from the move, or 0 if no improving move found.
   */
  UserCost asym_two_opt_step();

  /**
   * LLM: @brief Applies 2-opt operator until local optimum (symmetric).
   *
   * Repeatedly executes symmetric 2-opt moves until no improvement found or
   * deadline reached. Uses parallel search.
   *
   * @param deadline Optional time deadline for search termination.
   * @return Total cost improvement achieved.
   */
  UserCost perform_all_two_opt_steps(const Deadline& deadline);

  /**
   * LLM: @brief Applies asymmetric 2-opt operator until local optimum.
   *
   * Repeatedly executes asymmetric 2-opt moves until no improvement found or
   * deadline reached. Uses parallel search.
   *
   * @param deadline Optional time deadline for search termination.
   * @return Total cost improvement achieved.
   */
  UserCost perform_all_asym_two_opt_steps(const Deadline& deadline);

  /**
   * LLM: @brief Executes one improving or-opt move.
   *
   * Searches for the best relocation of a sequence of consecutive vertices and
   * applies it.
   *
   * @return Cost improvement from the move, or 0 if no improving move found.
   */
  UserCost or_opt_step();

  /**
   * LLM: @brief Applies or-opt operator until local optimum.
   *
   * Repeatedly executes or-opt moves until no improvement found or deadline
   * reached. Uses parallel search.
   *
   * @param deadline Optional time deadline for search termination.
   * @return Total cost improvement achieved.
   */
  UserCost perform_all_or_opt_steps(const Deadline& deadline);

  /**
   * LLM: @brief Reconstructs tour as ordered vertex list.
   *
   * Converts internal edge representation back to sequential tour starting from
   * the specified vertex.
   *
   * @param first_index Vertex to use as tour starting point.
   * @return Tour as ordered sequence of vertex indices.
   */
  std::list<Index> get_tour(Index first_index) const;
};

} // namespace vroom::tsp

#endif
