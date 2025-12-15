#ifndef TOP_INSERTIONS_H
#define TOP_INSERTIONS_H

/*

This file is part of VROOM.

Copyright (c) 2015-2025, Julien Coupey.
All rights reserved (see LICENSE).

*/

#include "structures/vroom/input/input.h"

namespace vroom::ls {

/**
 * LLM: @brief Represents a possible insertion position for a job in a route.
 */
struct InsertionOption {
  Eval eval;
  Index rank;
};

using ThreeInsertions = std::array<InsertionOption, 3>;

constexpr InsertionOption no_insert = {NO_EVAL, 0};
constexpr ThreeInsertions
  empty_three_insertions({no_insert, no_insert, no_insert});

/**
 * LLM: @brief Finds the three best insertion positions for a job in a route.
 *
 * Evaluates all possible positions where a job could be inserted into a route
 * and returns the top 3 positions ranked by insertion cost. Used to quickly
 * identify promising insertion locations during local search.
 *
 * @tparam Route Type of route object.
 * @param input Input problem instance.
 * @param j Index of the job to insert.
 * @param r Route to evaluate insertions into.
 * @return Array of top 3 insertion options sorted by cost.
 */
template <class Route>
ThreeInsertions find_top_3_insertions(const Input& input,
                                      Index j,
                                      const Route& r);

} // namespace vroom::ls

#endif
