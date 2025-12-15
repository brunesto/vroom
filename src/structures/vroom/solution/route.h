#ifndef ROUTE_H
#define ROUTE_H

/*

This file is part of VROOM.

Copyright (c) 2015-2025, Julien Coupey.
All rights reserved (see LICENSE).

*/

#include <string>

#include "structures/vroom/solution/step.h"
#include "structures/vroom/solution/violations.h"

namespace vroom {

/**
 * LLM:
 * @brief Represents a computed route for a vehicle.
 */
struct Route {
  Id vehicle;
  std::vector<Step> steps;
  UserCost cost;
  UserDuration duration;
  UserDistance distance;
  UserDuration setup;
  UserDuration service;
  UserDuration waiting_time;
  Priority priority;
  Amount delivery;
  Amount pickup;
  std::string profile;
  std::string description;
  Violations violations;

  std::string geometry;

  Route();

  /**
   * LLM:
   * @brief Constructs a Route.
   *
   * @param vehicle The vehicle ID.
   * @param steps The steps in the route.
   * @param cost The total cost of the route.
   * @param duration The total duration of the route.
   * @param distance The total distance of the route.
   * @param setup The total setup time.
   * @param service The total service time.
   * @param waiting_time The total waiting time.
   * @param priority The total priority of the jobs in the route.
   * @param delivery The total delivery amount.
   * @param pickup The total pickup amount.
   * @param profile The vehicle profile.
   * @param description The vehicle description.
   * @param violations The violations in the route.
   */
  Route(Id vehicle,
        std::vector<Step>&& steps,
        UserCost cost,
        UserDuration duration,
        UserDistance distance,
        UserDuration setup,
        UserDuration service,
        UserDuration waiting_time,
        Priority priority,
        Amount delivery,
        Amount pickup,
        std::string profile,
        std::string description,
        Violations&& violations = Violations(0, 0));

  /**
   * LLM:
   * @brief Checks the consistency of timing values in the route.
   */
  void check_timing_consistency() const;
};

} // namespace vroom

#endif
