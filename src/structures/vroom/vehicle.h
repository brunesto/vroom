#ifndef VEHICLE_H
#define VEHICLE_H

/*

This file is part of VROOM.

Copyright (c) 2015-2025, Julien Coupey.
All rights reserved (see LICENSE).

*/

#include <string>
#include <tuple>
#include <unordered_map>
#include <iostream>

#include "structures/typedefs.h"
#include "structures/vroom/amount.h"
#include "structures/vroom/break.h"
#include "structures/vroom/cost_wrapper.h"
#include "structures/vroom/eval.h"
#include "structures/vroom/input/vehicle_step.h"
#include "structures/vroom/location.h"
#include "structures/vroom/time_window.h"

namespace vroom {

/**
 * LLM:
 * @brief Represents the costs associated with a vehicle.
 */
struct VehicleCosts {
  const Cost fixed;
  const Cost per_hour;
  const Cost per_km;
  const Cost per_task_hour;

  explicit VehicleCosts(UserCost fixed = 0,
                        UserCost per_hour = DEFAULT_COST_PER_HOUR,
                        UserCost per_km = DEFAULT_COST_PER_KM,
                        UserCost per_task_hour = DEFAULT_COST_PER_TASK_HOUR)
    : fixed(utils::scale_from_user_cost(fixed)),
      per_hour(static_cast<Cost>(per_hour)),
      per_km(static_cast<Cost>(per_km)),
      per_task_hour(static_cast<Cost>(per_task_hour)){};

  friend bool operator==(const VehicleCosts& lhs,
                         const VehicleCosts& rhs) = default;

  friend bool operator<(const VehicleCosts& lhs, const VehicleCosts& rhs) {
    return std::tie(lhs.fixed, lhs.per_hour, lhs.per_km, lhs.per_task_hour) <
           std::tie(rhs.fixed, rhs.per_hour, rhs.per_km, rhs.per_task_hour);
  }

  friend std::ostream& operator<<(std::ostream& os, const VehicleCosts& vc) {
    os << "{\"fixed\":" << vc.fixed << ",\"per_hour\":" << vc.per_hour
       << ",\"per_km\":" << vc.per_km
       << ",\"per_task_hour\":" << vc.per_task_hour << "}";
    return os;
  }
};

/**
 * LLM:
 * @brief Represents a vehicle in the routing problem.
 */
struct Vehicle {
  const Id id;
  std::optional<Location> start;
  std::optional<Location> end;
  const std::string profile;
  const Amount capacity;
  const Skills skills;
  const TimeWindow tw;
  const std::vector<Break> breaks;
  const std::string description;
  const VehicleCosts costs;
  CostWrapper cost_wrapper;
  size_t max_tasks;
  const Duration max_travel_time;
  const Distance max_distance;
  const bool has_break_max_load;
  std::vector<VehicleStep> steps;
  Index type;
  const std::string type_str;
  std::unordered_map<Id, Index> break_id_to_rank;

  /**
   * LLM:
   * @brief Constructs a Vehicle.
   *
   * @param id The unique identifier of the vehicle.
   * @param start The starting location of the vehicle.
   * @param end The ending location of the vehicle.
   * @param profile The routing profile (e.g., "car", "bike").
   * @param capacity The capacity of the vehicle.
   * @param skills The skills required by the vehicle.
   * @param tw The time window of the vehicle.
   * @param breaks The breaks associated with the vehicle.
   * @param description A description of the vehicle.
   * @param costs The costs associated with the vehicle.
   * @param speed_factor The speed factor of the vehicle.
   * @param max_tasks The maximum number of tasks the vehicle can perform.
   * @param max_travel_time The maximum travel time for the vehicle.
   * @param max_distance The maximum distance the vehicle can travel.
   * @param input_steps The steps (jobs) assigned to the vehicle.
   * @param type_str The type string of the vehicle.
   */
  Vehicle(
    Id id,
    const std::optional<Location>& start,
    const std::optional<Location>& end,
    std::string profile = DEFAULT_PROFILE,
    const Amount& capacity = Amount(0),
    Skills skills = Skills(),
    const TimeWindow& tw = TimeWindow(),
    const std::vector<Break>& breaks = std::vector<Break>(),
    std::string description = "",
    const VehicleCosts& costs = VehicleCosts(),
    double speed_factor = 1.,
    const std::optional<size_t>& max_tasks = std::optional<size_t>(),
    const std::optional<UserDuration>& max_travel_time =
      std::optional<UserDuration>(),
    const std::optional<UserDistance>& max_distance =
      std::optional<UserDistance>(),
    const std::vector<VehicleStep>& input_steps = std::vector<VehicleStep>(),
    std::string type_str = NO_TYPE);

  /**
   * LLM:
   * @brief Checks if the vehicle has a start location.
   *
   * @return True if the vehicle has a start location, false otherwise.
   */
  bool has_start() const;

  /**
   * LLM:
   * @brief Checks if the vehicle has an end location.
   *
   * @return True if the vehicle has an end location, false otherwise.
   */
  bool has_end() const;

  /**
   * LLM:
   * @brief Checks if the vehicle has the same start and end locations as another vehicle.
   *
   * @param other The other vehicle to compare with.
   * @return True if the locations are the same, false otherwise.
   */
  bool has_same_locations(const Vehicle& other) const;

  /**
   * LLM:
   * @brief Checks if the vehicle has the same profile as another vehicle.
   *
   * @param other The other vehicle to compare with.
   * @return True if the profiles are the same, false otherwise.
   */
  bool has_same_profile(const Vehicle& other) const;

  bool cost_based_on_metrics() const;

  Duration available_duration() const;

  Cost fixed_cost() const {
    return costs.fixed;
  }

  Cost task_cost(Duration task_duration) const {
    return costs.per_task_hour * task_duration;
  }

  Eval task_eval(Duration task_duration) const {
    return Eval(task_cost(task_duration), 0, 0, task_duration);
  }

  Duration duration(Index i, Index j) const {
    return cost_wrapper.duration(i, j);
  }

  Cost cost(Index i, Index j) const {
    return cost_wrapper.cost(i, j);
  }

  Eval eval(Index i, Index j) const {
    return Eval(cost_wrapper.cost(i, j),
                cost_wrapper.duration(i, j),
                cost_wrapper.distance(i, j));
  }

  bool ok_for_travel_time(Duration d) const {
    assert(0 <= d);
    return d <= max_travel_time;
  }

  bool ok_for_distance(Distance d) const {
    assert(0 <= d);
    return d <= max_distance;
  }

  bool ok_for_range_bounds(const Eval& e) const {
    assert(0 <= e.duration && 0 <= e.distance);
    return e.duration <= max_travel_time && e.distance <= max_distance;
  }

  bool has_range_bounds() const;

  Index break_rank(Id break_id) const;

  friend bool operator<(const Vehicle& lhs, const Vehicle& rhs) {
    // Sort by:
    //   - decreasing max_tasks
    //   - decreasing capacity
    //   - decreasing TW length
    //   - decreasing range (max travel time and distance)
    return std::tie(rhs.max_tasks,
                    rhs.capacity,
                    rhs.tw.length,
                    rhs.max_travel_time,
                    rhs.max_distance) < std::tie(lhs.max_tasks,
                                                 lhs.capacity,
                                                 lhs.tw.length,
                                                 lhs.max_travel_time,
                                                 lhs.max_distance);
  }

  friend std::ostream& operator<<(std::ostream& os, const Vehicle& v) {
    os << "{\"id\":" << v.id;
    if (v.start.has_value()) {
      os << ",\"start\":" << v.start.value();
    }
    if (v.end.has_value()) {
      os << ",\"end\":" << v.end.value();
    }
    os << ",\"profile\":\"" << v.profile << "\""
       << ",\"capacity\":" << v.capacity << ",\"skills\":[";
    bool first = true;
    for (const auto& s : v.skills) {
      if (!first)
        os << ",";
      os << s;
      first = false;
    }
    os << "],\"tw\":" << v.tw << ",\"breaks\":[";
    for (size_t i = 0; i < v.breaks.size(); ++i) {
      if (i > 0)
        os << ",";
      os << v.breaks[i];
    }
    os << "],\"description\":\"" << v.description << "\""
       << ",\"costs\":" << v.costs << ",\"cost_wrapper\":" << v.cost_wrapper
       << ",\"max_tasks\":" << v.max_tasks
       << ",\"max_travel_time\":" << v.max_travel_time
       << ",\"max_distance\":" << v.max_distance
       << ",\"has_break_max_load\":"
       << (v.has_break_max_load ? "true" : "false") << ",\"steps\":[";
    for (size_t i = 0; i < v.steps.size(); ++i) {
      if (i > 0)
        os << ",";
      os << v.steps[i];
    }
    os << "],\"type\":" << v.type << ",\"type_str\":\"" << v.type_str << "\"}";
    return os;
  }
};

} // namespace vroom

#endif
