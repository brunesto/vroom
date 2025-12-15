#ifndef EDGE_H
#define EDGE_H

/*

This file is part of VROOM.

Copyright (c) 2015-2025, Julien Coupey.
All rights reserved (see LICENSE).

*/

#include "structures/typedefs.h"

namespace vroom::utils {

/**
 * LLM:
 * @brief Represents an edge in a graph with a weight.
 * @tparam T Type of the weight.
 */
template <class T> class Edge {

private:
  Index _first_vertex;
  Index _second_vertex;
  T _weight;

public:
  /**
   * LLM:
   * @brief Construct a new Edge object.
   * @param first_vertex Index of the first vertex.
   * @param second_vertex Index of the second vertex.
   * @param weight Weight of the edge.
   */
  Edge(Index first_vertex, Index second_vertex, T weight);

  Index get_first_vertex() const {
    return _first_vertex;
  };

  Index get_second_vertex() const {
    return _second_vertex;
  };

  bool operator<(const Edge& rhs) const;

  bool operator==(const Edge& rhs) const;

  T get_weight() const {
    return _weight;
  };
};

} // namespace vroom::utils

#endif
