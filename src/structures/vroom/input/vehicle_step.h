#ifndef VEHICLE_STEP_H
#define VEHICLE_STEP_H

/*

This file is part of VROOM.

Copyright (c) 2015-2025, Julien Coupey.
All rights reserved (see LICENSE).

*/

#include <iostream>

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

  friend std::ostream& operator<<(std::ostream& os, const ForcedService& fs) {
    os << "{";
    bool first = true;
    if (fs.at.has_value()) {
      os << "\"at\":" << fs.at.value();
      first = false;
    }
    if (fs.after.has_value()) {
      if (!first)
        os << ",";
      os << "\"after\":" << fs.after.value();
      first = false;
    }
    if (fs.before.has_value()) {
      if (!first)
        os << ",";
      os << "\"before\":" << fs.before.value();
    }
    os << "}";
    return os;
  }
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

  friend std::ostream& operator<<(std::ostream& os, const VehicleStep& vs) {
    os << "{\"id\":" << vs.id << ",\"type\":\"";
    switch (vs.type) {
    case STEP_TYPE::START:
      os << "START";
      break;
    case STEP_TYPE::JOB:
      os << "JOB";
      break;
    case STEP_TYPE::BREAK:
      os << "BREAK";
      break;
    case STEP_TYPE::END:
      os << "END";
      break;
    }
    os << "\"";
    if (vs.job_type.has_value()) {
      os << ",\"job_type\":\"";
      switch (vs.job_type.value()) {
      case JOB_TYPE::SINGLE:
        os << "SINGLE";
        break;
      case JOB_TYPE::PICKUP:
        os << "PICKUP";
        break;
      case JOB_TYPE::DELIVERY:
        os << "DELIVERY";
        break;
      }
      os << "\"";
    }
    os << ",\"forced_service\":" << vs.forced_service;
    os << ",\"rank\":" << vs.rank << "}";
    return os;
  }
};

} // namespace vroom

#endif
