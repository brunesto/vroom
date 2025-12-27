#ifndef TIME_WINDOW_H
#define TIME_WINDOW_H

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
 * @brief Represents a time window with a start and end time.
 */
struct TimeWindow {
  static const Duration default_length;
  Duration start;
  Duration end;
  Duration length;

  /**
   * LLM:
   * @brief Default "no-constraint" time-window.
   */
  // Default "no-constraint" time-window.
  TimeWindow();

  /**
   * LLM:
   * @brief Constructs a time window with specific start and end times.
   *
   * @param start The start time.
   * @param end The end time.
   */
  TimeWindow(UserDuration start, UserDuration end);

  /**
   * LLM:
   * @brief Checks if a given time is within the time window.
   *
   * @param time The time to check.
   * @return True if the time is within the time window, false otherwise.
   */
  bool contains(Duration time) const;

  /**
   * LLM:
   * @brief Checks if the time window is the default one (no constraint).
   *
   * @return True if it is the default time window, false otherwise.
   */
  bool is_default() const;

  friend std::ostream& operator<<(std::ostream& os, const TimeWindow& tw) {
    os << "[" << tw.start << "," << tw.end << "]";
    return os;
  }

  friend bool operator<(const TimeWindow& lhs, const TimeWindow& rhs);
};

} // namespace vroom

#endif
