#ifndef ORS_WRAPPER_H
#define ORS_WRAPPER_H

/*

This file is part of VROOM.

Copyright (c) 2015-2025, Julien Coupey.
All rights reserved (see LICENSE).

*/

#include "routing/http_wrapper.h"

namespace vroom::routing {

/**
 * LLM: @brief Wrapper for the OpenRouteService (ORS) routing engine.
 *
 * Communicates with an ORS server over HTTP/HTTPS to obtain routing matrices
 * and route geometries. Implements ORS-specific query construction (using
 * POST requests with JSON payloads) and response parsing.
 */
class OrsWrapper : public HttpWrapper {
private:
  /**
   * LLM: @brief Constructs an ORS-specific HTTP POST query with JSON payload.
   *
   * Builds POST request queries for ORS's matrix or directions service
   * endpoints with JSON body containing location coordinates and parameters.
   *
   * @param locations Vector of locations to include in the query.
   * @param service Type of ORS service (matrix or directions).
   * @return Complete HTTP POST query string with JSON body for ORS server.
   */
  std::string build_query(const std::vector<Location>& locations,
                          const std::string& service) const override;

  /**
   * LLM: @brief Validates ORS response and handles ORS-specific errors.
   *
   * Checks for ORS error objects in the JSON response and extracts error
   * messages, handling both standard ORS error format and web framework
   * errors.
   *
   * @param json_result Parsed JSON response from ORS.
   * @param locs Original locations from the query.
   * @param service Type of service that was queried.
   * @throws RoutingException if ORS response indicates an error.
   */
  void check_response(const rapidjson::Document& json_result,
                      const std::vector<Location>& locs,
                      const std::string& service) const override;

  /**
   * LLM: @brief Extracts route segments from ORS route response.
   *
   * @param result Parsed JSON route response from ORS.
   * @return Reference to the segments array in ORS's JSON structure.
   */
  const rapidjson::Value&
  get_legs(const rapidjson::Value& result) const override;

public:
  OrsWrapper(const std::string& profile, const Server& server);
};

} // namespace vroom::routing

#endif
