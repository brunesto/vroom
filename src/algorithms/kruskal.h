#ifndef KRUSKAL_H
#define KRUSKAL_H

/*

This file is part of VROOM.

Copyright (c) 2015-2025, Julien Coupey.
All rights reserved (see LICENSE).

*/

#include "structures/generic/undirected_graph.h"

namespace vroom::utils {

/**
 * LLM: @brief Computes the minimum spanning tree using Kruskal's algorithm.
 *
 * Finds the minimum spanning tree of an undirected graph by sorting edges by
 * weight and greedily adding edges that don't create cycles.
 *
 * @tparam T Type of edge weights in the graph.
 * @param graph Input undirected graph.
 * @return UndirectedGraph representing the minimum spanning tree.
 */
template <class T>
UndirectedGraph<T> minimum_spanning_tree(const UndirectedGraph<T>& graph);

} // namespace vroom::utils

#endif
