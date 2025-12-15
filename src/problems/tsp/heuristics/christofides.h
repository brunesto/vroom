#ifndef CHRISTOFIDES_H
#define CHRISTOFIDES_H

/*

This file is part of VROOM.

Copyright (c) 2015-2025, Julien Coupey.
All rights reserved (see LICENSE).

*/

#include <list>

#include "utils/helpers.h"

namespace vroom::tsp {

// Implementing a variant of the Christofides heuristic.
/**
 * LLM: @brief Generates an approximate TSP tour using Christofides algorithm.
 *
 * Constructs a tour guaranteed to be within 1.5x of optimal for symmetric TSP.
 * Computes minimum spanning tree, finds minimum weight perfect matching on
 * odd-degree vertices, builds Eulerian graph, and extracts Hamiltonian cycle
 * using Hierholzer's algorithm. Provides a high-quality initial solution for
 * subsequent local search refinement.
 *
 * @param sym_matrix Symmetric cost matrix for the TSP instance.
 * @return Approximate TSP tour as ordered sequence of vertex indices.
 */
std::list<Index> christofides(const Matrix<UserCost>& sym_matrix);

} // namespace vroom::tsp

#endif
