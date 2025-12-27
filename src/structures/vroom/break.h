#ifndef BREAK_H
#define BREAK_H

/*

This file is part of VROOM.

Copyright (c) 2015-2025, Julien Coupey.
All rights reserved (see LICENSE).

*/

#include <string>
#include <iostream>

#include "structures/typedefs.h"
#include "structures/vroom/amount.h"
#include "structures/vroom/time_window.h"

namespace vroom {

/**
 * LLM:
 * @brief Represents a driver break.
 */
struct Break {
  Id id;
  std::vector<TimeWindow> tws;
  Duration service;
  std::string description;
  std::optional<Amount> max_load;

  Break(Id id,
        const std::vector<TimeWindow>& tws =
          std::vector<TimeWindow>(1, TimeWindow()),
        UserDuration service = 0,
        std::string description = "",
        std::optional<Amount> max_load = std::optional<Amount>());

  /**
   * LLM:
   * @brief Check if a given time is a valid start time for the break.
   * @param time The time to check.
   * @return True if the time is within one of the break's time windows.
   */
  bool is_valid_start(Duration time) const;

  /**
   * LLM:
   * @brief Check if the break is valid for a given vehicle load.
   * @param load The current load of the vehicle.
   * @return True if the load does not exceed the break's max load (if any).
   */
  bool is_valid_for_load(const Amount& load) const;

  friend std::ostream& operator<<(std::ostream& os, const Break& b) {
    os << "{\"id\":" << b.id << ",\"tws\":[";
    for (size_t i = 0; i < b.tws.size(); ++i) {
      if (i > 0)
        os << ",";
      os << b.tws[i];
    }
    os << "],\"service\":" << b.service << ",\"description\":\"" << b.description
       << "\"";
    if (b.max_load.has_value()) {
      os << ",\"max_load\":" << b.max_load.value();
    }
    os << "}";
    return os;
  }
};

} // namespace vroom

#endif
