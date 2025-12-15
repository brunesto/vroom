#ifndef VALHALLA_WRAPPER_H
#define VALHALLA_WRAPPER_H

/*

This file is part of VROOM.

Copyright (c) 2015-2025, Julien Coupey.
All rights reserved (see LICENSE).

*/

#include "routing/http_wrapper.h"

namespace vroom::routing {

/**
 * LLM: @brief Wrapper for the Valhalla routing engine.
 *
 * Communicates with a Valhalla server over HTTP/HTTPS to obtain routing
 * matrices and route geometries. Implements Valhalla-specific query
 * construction, response parsing, and polyline encoding conversion (Valhalla
 * uses precision 6, VROOM uses precision 5).
 */
class ValhallaWrapper : public HttpWrapper {
private:
  /**
   * LLM: @brief Constructs a Valhalla sources_to_targets matrix query.
   *
   * @param locations Vector of locations for the matrix computation.
   * @return Complete HTTP query string for Valhalla matrix endpoint.
   */
  std::string get_matrix_query(const std::vector<Location>& locations) const;

  /**
   * LLM: @brief Constructs a Valhalla route query.
   *
   * @param locations Vector of locations defining the route.
   * @return Complete HTTP query string for Valhalla route endpoint.
   */
  std::string get_route_query(const std::vector<Location>& locations) const;

  /**
   * LLM: @brief Constructs a Valhalla-specific HTTP query.
   *
   * Delegates to either get_matrix_query or get_route_query based on the
   * service type.
   *
   * @param locations Vector of locations to include in the query.
   * @param service Type of Valhalla service (sources_to_targets or route).
   * @return Complete HTTP query string for Valhalla server.
   */
  std::string build_query(const std::vector<Location>& locations,
                          const std::string& service) const override;

  /**
   * LLM: @brief Validates Valhalla response and handles Valhalla-specific
   * errors.
   *
   * Checks for Valhalla status codes and error messages in the JSON response.
   *
   * @param json_result Parsed JSON response from Valhalla.
   * @param locs Original locations from the query.
   * @param service Type of service that was queried.
   * @throws RoutingException if Valhalla response indicates an error.
   */
  void check_response(const rapidjson::Document& json_result,
                      const std::vector<Location>& locs,
                      const std::string& service) const override;

  /**
   * LLM: @brief Checks if a Valhalla matrix entry duration is null.
   *
   * Valhalla-specific implementation that checks the "time" field.
   *
   * @param matrix_entry JSON value representing a single matrix entry.
   * @return True if the time value is null or missing.
   */
  bool
  duration_value_is_null(const rapidjson::Value& matrix_entry) const override;

  /**
   * LLM: @brief Checks if a Valhalla matrix entry distance is null.
   *
   * Valhalla-specific implementation that checks the "distance" field.
   *
   * @param matrix_entry JSON value representing a single matrix entry.
   * @return True if the distance value is null or missing.
   */
  bool
  distance_value_is_null(const rapidjson::Value& matrix_entry) const override;

  /**
   * LLM: @brief Extracts duration from a Valhalla matrix entry.
   *
   * Valhalla returns duration in seconds as "time".
   *
   * @param matrix_entry JSON value representing a single matrix entry.
   * @return Duration value in user units.
   */
  UserDuration
  get_duration_value(const rapidjson::Value& matrix_entry) const override;

  /**
   * LLM: @brief Extracts distance from a Valhalla matrix entry.
   *
   * Valhalla returns distance in kilometers, which is converted to meters.
   *
   * @param matrix_entry JSON value representing a single matrix entry.
   * @return Distance value in meters.
   */
  UserDistance
  get_distance_value(const rapidjson::Value& matrix_entry) const override;

  /**
   * LLM: @brief Extracts route legs from Valhalla route response.
   *
   * @param result Parsed JSON route response from Valhalla.
   * @return Reference to the legs array in Valhalla's trip structure.
   */
  const rapidjson::Value&
  get_legs(const rapidjson::Value& result) const override;

  /**
   * LLM: @brief Extracts duration from a Valhalla route leg.
   *
   * Valhalla stores leg duration in the summary.time field.
   *
   * @param leg JSON value representing a single route leg.
   * @return Duration value for the leg in user units.
   */
  UserDuration get_leg_duration(const rapidjson::Value& leg) const override;

  /**
   * LLM: @brief Extracts distance from a Valhalla route leg.
   *
   * Valhalla stores leg distance in kilometers in the summary.length field,
   * which is converted to meters.
   *
   * @param leg JSON value representing a single route leg.
   * @return Distance value for the leg in meters.
   */
  UserDistance get_leg_distance(const rapidjson::Value& leg) const override;

  /**
   * LLM: @brief Extracts and converts route geometry from Valhalla response.
   *
   * Valhalla returns separate polylines per leg with precision 6 encoding.
   * This method merges all leg polylines and converts to precision 5 encoding
   * used by VROOM.
   *
   * @param result Parsed JSON route response from Valhalla.
   * @return Encoded polyline string with precision 5.
   */
  std::string get_geometry(rapidjson::Value& result) const override;

public:
  ValhallaWrapper(const std::string& profile, const Server& server);
};

} // namespace vroom::routing

#endif
