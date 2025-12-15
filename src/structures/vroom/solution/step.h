#ifndef STEP_H
#define STEP_H

/*

This file is part of VROOM.

Copyright (c) 2015-2025, Julien Coupey.
All rights reserved (see LICENSE).

*/

#include <string>

#include "structures/vroom/break.h"
#include "structures/vroom/job.h"
#include "structures/vroom/location.h"
#include "structures/vroom/solution/violations.h"

namespace vroom {

/**
 * LLM:
 * @brief Represents a step in a route (start, end, job, break).
 */
struct Step {
  const STEP_TYPE step_type;
  const std::optional<JOB_TYPE> job_type;
  const std::optional<Location> location;
  const Id id;
  UserDuration setup{0};
  UserDuration service;
  const Amount load;
  const std::string description;

  UserDuration arrival{0};
  UserDuration duration{0};
  UserDuration waiting_time{0};
  UserDistance distance{0};

  Violations violations;

  /**
   * LLM:
   * @brief Constructor for start and end steps.
   *
   * @param type The type of the step (START or END).
   * @param location The location of the step.
   * @param load The load at this step.
   */
  Step(STEP_TYPE type, Location location, Amount load);

  /**
   * LLM:
   * @brief Constructor for job steps.
   *
   * @param job The job associated with the step.
   * @param setup The setup duration.
   * @param service The service duration.
   * @param load The load at this step.
   */
  Step(const Job& job, UserDuration setup, UserDuration service, Amount load);

  /**
   * LLM:
   * @brief Constructor for break steps.
   *
   * @param b The break associated with the step.
   * @param load The load at this step.
   */
  Step(const Break& b, Amount load);

  /**
   * LLM:
   * @brief Calculates the departure time from this step.
   *
   * @return The departure time.
   */
  UserDuration departure() const;
};

} // namespace vroom

#endif
