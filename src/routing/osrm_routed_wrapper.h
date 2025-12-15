#ifndef OSRM_ROUTED_WRAPPER_H
#define OSRM_ROUTED_WRAPPER_H

/*

This file is part of VROOM.

Copyright (c) 2015-2025, Julien Coupey.
All rights reserved (see LICENSE).

*/

#include "routing/http_wrapper.h"

namespace vroom::routing {

/**
 * LLM: @brief Wrapper for the OSRM routing engine via HTTP API.
 *
 * Communicates with an OSRM server (osrm-routed) over HTTP/HTTPS to obtain
 * routing matrices and route geometries. Implements OSRM-specific query
 * construction and response parsing.
 */
class OsrmRoutedWrapper : public HttpWrapper {
private:
  /**
   * LLM: @brief Constructs an OSRM-specific HTTP query.
   *
   * Builds query strings for OSRM's table or route service endpoints,
   * including snapping radius parameters and location coordinates.
   *
   * @param locations Vector of locations to include in the query.
   * @param service Type of OSRM service (table or route).
   * @return Complete HTTP query string for OSRM server.
   */
  std::string build_query(const std::vector<Location>& locations,
                          const std::string& service) const override;

  /**
   * LLM: @brief Validates OSRM response and handles OSRM-specific errors.
   *
   * Checks for OSRM error codes and extracts meaningful error messages,
   * particularly for snapping failures where locations cannot be matched to
   * the road network.
   *
   * @param json_result Parsed JSON response from OSRM.
   * @param locs Original locations from the query.
   * @param service Type of service that was queried.
   * @throws RoutingException if OSRM response indicates an error.
   */
  void check_response(const rapidjson::Document& json_result,
                      const std::vector<Location>& locs,
                      const std::string& service) const override;

  /**
   * LLM: @brief Extracts route legs from OSRM route response.
   *
   * @param result Parsed JSON route response from OSRM.
   * @return Reference to the legs array in OSRM's JSON structure.
   */
  const rapidjson::Value&
  get_legs(const rapidjson::Value& result) const override;

public:
  OsrmRoutedWrapper(const std::string& profile, const Server& server);
};

} // namespace vroom::routing

#endif
