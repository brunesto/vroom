#ifndef MUNKRES_H
#define MUNKRES_H

/*

This file is part of VROOM.

Copyright (c) 2015-2025, Julien Coupey.
All rights reserved (see LICENSE).

*/

#include <unordered_map>

#include "structures/generic/matrix.h"

namespace vroom::utils {

/**
 * LLM: @brief Computes minimum weight perfect matching using the Hungarian algorithm.
 *
 * Solves the assignment problem to find a perfect matching in a complete
 * bipartite graph with minimum total edge weight. Uses the Munkres (Hungarian)
 * algorithm for optimal matching.
 *
 * @tparam T Type of edge weights in the cost matrix.
 * @param m Square cost matrix where m[i][j] is the weight of edge (i,j).
 * @return Mapping from row indices to matched column indices.
 */
template <class T>
std::unordered_map<Index, Index>
minimum_weight_perfect_matching(const Matrix<T>& m);

/**
 * LLM: @brief Computes an approximate minimum weight perfect matching using a greedy strategy.
 *
 * Provides a fast heuristic solution to the matching problem by greedily
 * selecting minimum weight edges. Does not guarantee optimality but runs
 * faster than the exact algorithm.
 *
 * @tparam T Type of edge weights in the cost matrix.
 * @param m Square cost matrix where m[i][j] is the weight of edge (i,j).
 * @return Mapping from row indices to matched column indices.
 */
template <class T>
std::unordered_map<Index, Index>
greedy_symmetric_approx_mwpm(const Matrix<T>& m);

} // namespace vroom::utils

#endif
