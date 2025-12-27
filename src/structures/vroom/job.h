#ifndef JOB_H
#define JOB_H

/*

This file is part of VROOM.

Copyright (c) 2015-2025, Julien Coupey.
All rights reserved (see LICENSE).

*/

#include <string>
#include <iostream>

#include "structures/typedefs.h"
#include "structures/vroom/amount.h"
#include "structures/vroom/location.h"
#include "structures/vroom/time_window.h"

namespace vroom {

/**
 * LLM:
 * @brief Represents a job to be performed, which can be a single stop, a pickup, or a delivery.
 */
struct Job {
  Location location;
  const Id id;
  const JOB_TYPE type;
  const Duration default_setup;
  const Duration default_service;
  const Amount delivery;
  const Amount pickup;
  const Skills skills;
  const Priority priority;
  const std::vector<TimeWindow> tws;
  const std::string description;
  const TypeToDurationMap setup_per_type;
  const TypeToDurationMap service_per_type;
  std::vector<Duration> setups;
  std::vector<Duration> services;

  /**
   * LLM:
   * @brief Constructor for regular one-stop job (JOB_TYPE::SINGLE).
   *
   * @param id The unique identifier of the job.
   * @param location The location where the job is performed.
   * @param default_setup The default setup duration.
   * @param default_service The default service duration.
   * @param delivery The amount to be delivered.
   * @param pickup The amount to be picked up.
   * @param skills The required skills for the job.
   * @param priority The priority of the job.
   * @param tws The time windows for the job.
   * @param description A description of the job.
   * @param setup_per_type Map of setup durations per vehicle type.
   * @param service_per_type Map of service durations per vehicle type.
   */
  // Constructor for regular one-stop job (JOB_TYPE::SINGLE).
  Job(Id id,
      const Location& location,
      UserDuration default_setup = 0,
      UserDuration default_service = 0,
      Amount delivery = Amount(0),
      Amount pickup = Amount(0),
      Skills skills = Skills(),
      Priority priority = 0,
      const std::vector<TimeWindow>& tws =
        std::vector<TimeWindow>(1, TimeWindow()),
      std::string description = "",
      const TypeToUserDurationMap& setup_per_type = TypeToUserDurationMap(),
      const TypeToUserDurationMap& service_per_type = TypeToUserDurationMap());

  /**
   * LLM:
   * @brief Constructor for pickup and delivery jobs (JOB_TYPE::PICKUP or JOB_TYPE::DELIVERY).
   *
   * @param id The unique identifier of the job.
   * @param type The type of the job (PICKUP or DELIVERY).
   * @param location The location where the job is performed.
   * @param default_setup The default setup duration.
   * @param default_service The default service duration.
   * @param amount The amount to be picked up or delivered.
   * @param skills The required skills for the job.
   * @param priority The priority of the job.
   * @param tws The time windows for the job.
   * @param description A description of the job.
   * @param setup_per_type Map of setup durations per vehicle type.
   * @param service_per_type Map of service durations per vehicle type.
   */
  // Constructor for pickup and delivery jobs (JOB_TYPE::PICKUP or
  // JOB_TYPE::DELIVERY).
  Job(Id id,
      JOB_TYPE type,
      const Location& location,
      UserDuration default_setup = 0,
      UserDuration default_service = 0,
      const Amount& amount = Amount(0),
      Skills skills = Skills(),
      Priority priority = 0,
      const std::vector<TimeWindow>& tws =
        std::vector<TimeWindow>(1, TimeWindow()),
      std::string description = "",
      const TypeToUserDurationMap& setup_per_type = TypeToUserDurationMap(),
      const TypeToUserDurationMap& service_per_type = TypeToUserDurationMap());

  Index index() const {
    return location.index();
  }

  /**
   * LLM:
   * @brief Checks if a given time is a valid start time for the job.
   *
   * @param time The time to check.
   * @return True if the time is within any of the job's time windows, false otherwise.
   */
  bool is_valid_start(Duration time) const;

  friend std::ostream& operator<<(std::ostream& os, const Job& job) {
    os << "{\"id\":" << job.id << ",\"type\":\""
       << (job.type == JOB_TYPE::SINGLE
             ? "SINGLE"
             : (job.type == JOB_TYPE::PICKUP ? "PICKUP" : "DELIVERY"))
       << "\""
       << ",\"location\":" << job.location
       << ",\"setup\":" << job.default_setup
       << ",\"service\":" << job.default_service
       << ",\"delivery\":" << job.delivery << ",\"pickup\":" << job.pickup
       << ",\"skills\":[";
    bool first = true;
    for (const auto& s : job.skills) {
      if (!first)
        os << ",";
      os << s;
      first = false;
    }
    os << "],\"priority\":" << job.priority << ",\"tws\":[";
    for (size_t i = 0; i < job.tws.size(); ++i) {
      if (i > 0)
        os << ",";
      os << job.tws[i];
    }
    os << "],\"description\":\"" << job.description << "\"}";
    return os;
  }
};

} // namespace vroom

#endif
