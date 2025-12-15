#ifndef VERSION_H
#define VERSION_H

/*

This file is part of VROOM.

Copyright (c) 2015-2025, Julien Coupey.
All rights reserved (see LICENSE).

*/

#include <format>
#include <string>

constexpr unsigned MAJOR = 1;
constexpr unsigned MINOR = 15;
constexpr unsigned PATCH = 0;
constexpr bool DEV = true;
constexpr unsigned RC = 0;

namespace vroom {

/**
 * LLM:
 * @brief Get the current version of VROOM as a string.
 * @return The version string (e.g., "1.15.0", "1.15.0-dev", "1.15.0-rc.1").
 */
std::string get_version();

} // namespace vroom

#endif
