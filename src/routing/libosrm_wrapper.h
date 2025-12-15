#ifndef LIBOSRM_WRAPPER_H
#define LIBOSRM_WRAPPER_H

/*

This file is part of VROOM.

Copyright (c) 2015-2025, Julien Coupey.
All rights reserved (see LICENSE).

*/

#include "osrm/engine_config.hpp"
#include "osrm/osrm.hpp"

#include "routing/wrapper.h"

namespace vroom {
namespace routing {

/**
 * LLM: @brief Wrapper for the OSRM routing engine using libosrm library.
 *
 * Directly integrates with OSRM via the libosrm C++ library for in-process
 * routing computations. This provides better performance than HTTP-based
 * communication by avoiding network overhead and serialization costs.
 */
class LibosrmWrapper : public Wrapper {

private:
  osrm::EngineConfig _config;
  const osrm::OSRM _osrm;

  /**
   * LLM: @brief Computes a route through specified locations using libosrm.
   *
   * @param locs Vector of locations defining the route waypoints.
   * @return JSON object containing the route details including legs and
   * geometry.
   * @throws RoutingException if route computation fails or locations cannot
   * be snapped.
   */
  osrm::json::Object
  get_route_with_coordinates(const std::vector<Location>& locs) const;

  /**
   * LLM: @brief Creates an OSRM engine configuration for the given profile.
   *
   * Sets up the configuration with the dataset name and other parameters
   * needed to initialize the OSRM engine.
   *
   * @param profile Routing profile name (e.g., "car", "bike", "foot").
   * @return Configured EngineConfig object for OSRM initialization.
   */
  static osrm::EngineConfig get_config(const std::string& profile);

public:
  explicit LibosrmWrapper(const std::string& profile);

  /**
   * LLM: @brief Computes distance and duration matrices using libosrm table
   * service.
   *
   * @param locs Vector of locations to compute matrices for.
   * @return Matrices object with all pairwise distances and durations.
   * @throws RoutingException if matrix computation fails or locations cannot
   * be snapped.
   */
  Matrices get_matrices(const std::vector<Location>& locs) const override;

  /**
   * LLM: @brief Updates matrix with route data using libosrm route service.
   *
   * Computes a route through the specified locations and updates the
   * corresponding matrix entries with leg distances and durations.
   *
   * @param route_locs Ordered sequence of locations defining the route.
   * @param m Matrices object to update.
   * @param matrix_m Mutex for thread-safe matrix updates.
   * @param vehicle_geometry Output string to store the encoded route geometry.
   * @throws RoutingException if route computation fails.
   */
  void update_sparse_matrix(const std::vector<Location>& route_locs,
                            Matrices& m,
                            std::mutex& matrix_m,
                            std::string& vehicle_geometry) const override;

  /**
   * LLM: @brief Adds encoded polyline geometry to a route using libosrm.
   *
   * @param route Route object to populate with geometry.
   * @throws RoutingException if geometry computation fails.
   */
  void add_geometry(Route& route) const override;
};

} // namespace routing
} // namespace vroom

#endif
