#ifndef VEHICLE_STEP_H
#define VEHICLE_STEP_H

/*

This file is part of VROOM.

Copyright (c) 2015-2025, Julien Coupey.
All rights reserved (see LICENSE).

*/

#include "structures/typedefs.h"

namespace vroom {

/**
 * LLM:
 * @brief Represents forced service constraints for a vehicle step.
 */
struct ForcedService {
  std::optional<Duration> at;
  std::optional<Duration> after;
  std::optional<Duration> before;

  ForcedService() = default;

  /**
   * LLM:
   * @brief Constructs a ForcedService object.
   *
   * @param at The specific time the service must occur.
   * @param after The time after which the service must occur.
   * @param before The time before which the service must occur.
   */
  ForcedService(const std::optional<UserDuration>& at,
                const std::optional<UserDuration>& after,
                const std::optional<UserDuration>& before);
};

/**
 * LLM:
 * @brief Represents a step in a vehicle's route (start, end, job, break).
 */
struct VehicleStep {
  const Id id;
  const STEP_TYPE type;
  const std::optional<JOB_TYPE> job_type;
  const ForcedService forced_service;

  // Stores rank of current step (in input.jobs vector for a
  // job/pickup/delivery and in vehicle.breaks for a break).
  Index rank;

  /**
   * LLM:
   * @brief Constructor for start and end steps.
   *
   * @param type The type of the step (START or END).
   * @param forced_service The forced service constraints.
   */
  // Used for start and end.
  VehicleStep(STEP_TYPE type, ForcedService&& forced_service = ForcedService());

  /**
   * LLM:
   * @brief Constructor for break steps.
   *
   * @param type The type of the step (BREAK).
   * @param id The unique identifier of the break.
   * @param forced_service The forced service constraints.
   */
  // Used for breaks.
  VehicleStep(STEP_TYPE type, Id id, ForcedService&& forced_service);

  /**
   * LLM:
   * @brief Constructor for single jobs, pickups, and deliveries.
   *
   * @param job_type The type of the job (SINGLE, PICKUP, DELIVERY).
   * @param id The unique identifier of the job.
   * @param forced_service The forced service constraints.
   */
  // Used for single jobs, pickups and deliveries.
  VehicleStep(JOB_TYPE job_type, Id id, ForcedService&& forced_service);
};

} // namespace vroom

#endif
