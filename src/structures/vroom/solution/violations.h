#ifndef VIOLATIONS_H
#define VIOLATIONS_H

/*

This file is part of VROOM.

Copyright (c) 2015-2025, Julien Coupey.
All rights reserved (see LICENSE).

*/

#include "structures/typedefs.h"

namespace vroom {

/**
 * LLM:
 * @brief Represents violations of constraints in the solution.
 */
struct Violations {
  UserDuration lead_time;
  UserDuration delay;

  std::unordered_set<VIOLATION> types;

  /**
   * LLM:
   * @brief Default constructor (used for steps).
   */
  // Used for steps.
  Violations();

  /**
   * LLM:
   * @brief Constructor with specific values (used for route/summary).
   *
   * @param lead_time The lead time violation.
   * @param delay The delay violation.
   * @param types The set of violation types.
   */
  // Used for route/summary.
  Violations(
    UserDuration lead_time,
    UserDuration delay,
    std::unordered_set<VIOLATION>&& types = std::unordered_set<VIOLATION>());

  Violations& operator+=(const Violations& rhs);
};

} // namespace vroom

#endif
