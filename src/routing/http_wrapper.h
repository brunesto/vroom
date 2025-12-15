#ifndef HTTP_WRAPPER_H
#define HTTP_WRAPPER_H

/*

This file is part of VROOM.

Copyright (c) 2015-2025, Julien Coupey.
All rights reserved (see LICENSE).

*/
#include "../include/rapidjson/include/rapidjson/document.h"

#include "routing/wrapper.h"
#include "structures/typedefs.h"
#include "utils/helpers.h"

namespace vroom::routing {

/**
 * LLM: @brief Base class for HTTP-based routing engine wrappers.
 *
 * Provides common HTTP/HTTPS communication functionality for routing engines
 * that expose REST APIs (OSRM, ORS, Valhalla). Handles query construction,
 * HTTP requests, response parsing, and matrix/route extraction.
 */
class HttpWrapper : public Wrapper {
private:
  /**
   * LLM: @brief Sends an HTTP query and receives the response.
   *
   * @param query Complete HTTP query string including headers.
   * @return JSON response body extracted from the HTTP response.
   * @throws RoutingException if connection fails or response is invalid.
   */
  std::string send_then_receive(const std::string& query) const;

  /**
   * LLM: @brief Sends an HTTPS query over SSL and receives the response.
   *
   * @param query Complete HTTPS query string including headers.
   * @return JSON response body extracted from the HTTPS response.
   * @throws RoutingException if SSL connection fails or response is invalid.
   */
  std::string ssl_send_then_receive(const std::string& query) const;

  static const std::string HTTPS_PORT;

protected:
  const Server _server;
  const std::string _matrix_service;
  const std::string _matrix_durations_key;
  const std::string _matrix_distances_key;
  const std::string _route_service;
  const std::string _routing_args;

  HttpWrapper(const std::string& profile,
              Server server,
              std::string matrix_service,
              std::string matrix_durations_key,
              std::string matrix_distances_key,
              std::string route_service,
              std::string routing_args);

  /**
   * LLM: @brief Executes an HTTP or HTTPS query based on the server port.
   *
   * Automatically selects HTTP or HTTPS based on whether the server port
   * is 443 (HTTPS) or another port (HTTP).
   *
   * @param query Complete query string including headers.
   * @return JSON response body.
   * @throws RoutingException if connection fails or response is invalid.
   */
  std::string run_query(const std::string& query) const;

  /**
   * LLM: @brief Parses JSON response from routing engine.
   *
   * @param json_result Output document to store parsed JSON.
   * @param json_content Raw JSON string to parse.
   * @throws RoutingException if JSON parsing fails.
   */
  static void parse_response(rapidjson::Document& json_result,
                             const std::string& json_content);

  /**
   * LLM: @brief Constructs a routing engine-specific query string.
   *
   * Each derived class implements this to create the appropriate query format
   * for its routing engine (OSRM, ORS, or Valhalla).
   *
   * @param locations Vector of locations to include in the query.
   * @param service Type of service to query (matrix or route).
   * @return Complete HTTP query string including headers.
   */
  virtual std::string build_query(const std::vector<Location>& locations,
                                  const std::string& service) const = 0;

  /**
   * LLM: @brief Validates the routing engine response for errors.
   *
   * Each derived class implements this to check for engine-specific error
   * conditions in the JSON response.
   *
   * @param json_result Parsed JSON response from routing engine.
   * @param locs Original locations from the query.
   * @param service Type of service that was queried.
   * @throws RoutingException if the response indicates an error.
   */
  virtual void check_response(const rapidjson::Document& json_result,
                              const std::vector<Location>& locs,
                              const std::string& service) const = 0;

  Matrices get_matrices(const std::vector<Location>& locs) const override;

  void update_sparse_matrix(const std::vector<Location>& route_locs,
                            Matrices& m,
                            std::mutex& matrix_m,
                            std::string& vehicle_geometry) const override;

  /**
   * LLM: @brief Checks if a matrix entry duration value is null/missing.
   *
   * @param matrix_entry JSON value representing a single matrix entry.
   * @return True if the duration value is null or missing.
   */
  virtual bool
  duration_value_is_null(const rapidjson::Value& matrix_entry) const {
    // Same implementation for both OSRM and ORS.
    return matrix_entry.IsNull();
  }

  /**
   * LLM: @brief Checks if a matrix entry distance value is null/missing.
   *
   * @param matrix_entry JSON value representing a single matrix entry.
   * @return True if the distance value is null or missing.
   */
  virtual bool
  distance_value_is_null(const rapidjson::Value& matrix_entry) const {
    // Same implementation for both OSRM and ORS.
    return matrix_entry.IsNull();
  }

  /**
   * LLM: @brief Extracts duration value from a matrix entry.
   *
   * @param matrix_entry JSON value representing a single matrix entry.
   * @return Duration value rounded to user units.
   */
  virtual UserDuration
  get_duration_value(const rapidjson::Value& matrix_entry) const {
    // Same implementation for both OSRM and ORS.
    return utils::round<UserDuration>(matrix_entry.GetDouble());
  }

  /**
   * LLM: @brief Extracts distance value from a matrix entry.
   *
   * @param matrix_entry JSON value representing a single matrix entry.
   * @return Distance value rounded to user units.
   */
  virtual UserDistance
  get_distance_value(const rapidjson::Value& matrix_entry) const {
    // Same implementation for both OSRM and ORS.
    return utils::round<UserDistance>(matrix_entry.GetDouble());
  }

  /**
   * LLM: @brief Extracts the legs array from a route response.
   *
   * Each derived class implements this to navigate the engine-specific JSON
   * structure to locate the route legs data.
   *
   * @param result Parsed JSON route response.
   * @return Reference to the JSON array containing route leg information.
   */
  virtual const rapidjson::Value&
  get_legs(const rapidjson::Value& result) const = 0;

  /**
   * LLM: @brief Extracts duration value from a single route leg.
   *
   * @param leg JSON value representing a single route leg.
   * @return Duration value for the leg rounded to user units.
   */
  virtual UserDuration get_leg_duration(const rapidjson::Value& leg) const {
    // Same implementation for both OSRM and ORS.
    assert(leg.HasMember("duration"));
    return utils::round<UserDuration>(leg["duration"].GetDouble());
  }

  /**
   * LLM: @brief Extracts distance value from a single route leg.
   *
   * @param leg JSON value representing a single route leg.
   * @return Distance value for the leg rounded to user units.
   */
  virtual UserDistance get_leg_distance(const rapidjson::Value& leg) const {
    // Same implementation for both OSRM and ORS.
    assert(leg.HasMember("distance"));
    return utils::round<UserDistance>(leg["distance"].GetDouble());
  }

  /**
   * LLM: @brief Extracts encoded route geometry from a route response.
   *
   * @param result Parsed JSON route response.
   * @return Encoded polyline string representing the route geometry.
   */
  virtual std::string get_geometry(rapidjson::Value& result) const {
    // Same implementation for both OSRM and ORS.
    return result["routes"][0]["geometry"].GetString();
  }

  void add_geometry(Route& route) const override;
};

} // namespace vroom::routing

#endif
