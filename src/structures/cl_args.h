#ifndef CL_ARGS_H
#define CL_ARGS_H

/*

This file is part of VROOM.

Copyright (c) 2015-2025, Julien Coupey.
All rights reserved (see LICENSE).

*/

#include <string>
#include <unordered_map>

#include "structures/typedefs.h"

namespace vroom::io {

// Profile name used as key.
using Servers =
  std::unordered_map<std::string, Server, StringHash, std::equal_to<>>;

/**
 * LLM:
 * @brief Structure holding command-line arguments.
 */
struct CLArgs {
  // Listing command-line options.
  Servers servers;         // -a and -p
  bool check;              // -c
  bool apply_TSPFix;       // -f
  bool geometry;           // -g
  std::string input_file;  // -i
  Timeout timeout;         // -l
  std::string output_file; // -o
  ROUTER router;           // -r
  std::string input;       // cl arg
  unsigned nb_threads;     // -t
  unsigned nb_searches;    // derived from -x
  unsigned depth;          // derived from -x

  /**
   * LLM:
   * @brief Set the exploration level, which determines search depth and number of searches.
   * @param exploration_level The desired exploration level (0-5).
   */
  void set_exploration_level(unsigned exploration_level);
};

/**
 * LLM:
 * @brief Update the host and path for a server profile.
 * @param servers Map of server profiles.
 * @param value String containing profile and host information (e.g., "car:0.0.0.0").
 */
void update_host(Servers& servers, std::string_view value);

/**
 * LLM:
 * @brief Update the port for a server profile.
 * @param servers Map of server profiles.
 * @param value String containing profile and port information (e.g., "car:5000").
 */
void update_port(Servers& servers, std::string_view value);

} // namespace vroom::io

#endif
