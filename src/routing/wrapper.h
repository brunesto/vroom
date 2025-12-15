#ifndef WRAPPER_H
#define WRAPPER_H

/*

This file is part of VROOM.

Copyright (c) 2015-2025, Julien Coupey.
All rights reserved (see LICENSE).

*/

#include <mutex>
#include <thread>
#include <vector>

#include "structures/generic/matrix.h"
#include "structures/vroom/location.h"
#include "structures/vroom/matrices.h"
#include "structures/vroom/solution/route.h"
#include "structures/vroom/vehicle.h"
#include "utils/exception.h"

namespace vroom::routing {

/**
 * LLM: @brief Abstract base class for routing engine wrappers.
 *
 * Provides a common interface for different routing engines (OSRM, ORS,
 * Valhalla, etc.) to compute distance/duration matrices and route geometries.
 * Each derived class implements the specifics of communicating with its
 * respective routing engine.
 */
class Wrapper {

public:
  std::string profile;

  /**
   * LLM: @brief Computes complete distance and duration matrices for all
   * location pairs.
   *
   * @param locs Vector of locations to compute matrices for.
   * @return Matrices object containing distance and duration values for all
   * location pairs.
   * @throws RoutingException if routes cannot be found or routing engine
   * returns an error.
   */
  virtual Matrices get_matrices(const std::vector<Location>& locs) const = 0;

  /**
   * LLM: @brief Computes sparse matrices using individual route queries for
   * each vehicle.
   *
   * Instead of computing a full NxN matrix, this method queries only the
   * specific route segments needed by each vehicle. Processes vehicles in
   * parallel for efficiency.
   *
   * @param locs Vector of all locations involved in the problem.
   * @param vehicles Vector of vehicles with their assigned routes.
   * @param jobs Vector of jobs to be performed.
   * @param vehicles_geometry Output vector to store route geometries for each
   * vehicle.
   * @return Matrices object with only the required matrix entries populated.
   * @throws RoutingException if routes cannot be found or routing engine
   * returns an error.
   */
  Matrices
  get_sparse_matrices(const std::vector<Location>& locs,
                      const std::vector<Vehicle>& vehicles,
                      const std::vector<Job>& jobs,
                      std::vector<std::string>& vehicles_geometry) const {
    const std::size_t m_size = locs.size();
    Matrices m(m_size);

    std::exception_ptr ep = nullptr;
    std::mutex ep_m;
    std::mutex matrix_m;

    auto run_on_vehicle_at_rank =
      [this, &vehicles, &jobs, &matrix_m, &m, &vehicles_geometry, &ep_m, &ep](
        Index v_rank) {
        try {
          const Vehicle& v = vehicles[v_rank];

          std::vector<Location> route_locs;
          route_locs.reserve(v.steps.size());

          bool has_job_steps = false;
          for (const auto& step : v.steps) {
            switch (step.type) {
              using enum STEP_TYPE;
            case START:
              if (v.has_start()) {
                route_locs.push_back(v.start.value());
              }
              break;
            case END:
              if (v.has_end()) {
                route_locs.push_back(v.end.value());
              }
              break;
            case BREAK:
              break;
            case JOB:
              has_job_steps = true;
              route_locs.push_back(jobs[step.rank].location);
              break;
            }
          }

          if (has_job_steps) {
            assert(route_locs.size() >= 2);

            this->update_sparse_matrix(route_locs,
                                       m,
                                       matrix_m,
                                       vehicles_geometry[v_rank]);
          }
        } catch (...) {
          const std::scoped_lock<std::mutex> lock(ep_m);
          ep = std::current_exception();
        }
      };

    std::vector<std::jthread> vehicles_threads;
    vehicles_threads.reserve(vehicles.size());

    for (Index v_rank = 0; v_rank < vehicles.size(); ++v_rank) {
      if (vehicles[v_rank].profile == this->profile) {
        vehicles_threads.emplace_back(run_on_vehicle_at_rank, v_rank);
      }
    }

    for (auto& t : vehicles_threads) {
      t.join();
    }

    if (ep != nullptr) {
      std::rethrow_exception(ep);
    }

    return m;
  };

  // Updates matrices with data from a single route request and stores
  // corresponding route geometry.
  /**
   * LLM: @brief Updates matrix entries with routing data for a single vehicle
   * route.
   *
   * Queries the routing engine for a specific sequence of locations and
   * updates the corresponding matrix entries. Thread-safe for concurrent
   * matrix updates.
   *
   * @param route_locs Ordered sequence of locations defining the route.
   * @param m Matrices object to update with distance/duration values.
   * @param matrix_m Mutex to synchronize concurrent matrix updates.
   * @param vehicle_geometry Output string to store the encoded route geometry.
   * @throws RoutingException if the route cannot be computed.
   */
  virtual void update_sparse_matrix(const std::vector<Location>& route_locs,
                                    Matrices& m,
                                    std::mutex& matrix_m,
                                    std::string& vehicle_geometry) const = 0;

  /**
   * LLM: @brief Adds route geometry to a solution route.
   *
   * Queries the routing engine to obtain the encoded polyline geometry for
   * the route's sequence of steps and stores it in the route object.
   *
   * @param route Route object to populate with geometry information.
   * @throws RoutingException if the route geometry cannot be computed.
   */
  virtual void add_geometry(Route& route) const = 0;

  virtual ~Wrapper() = default;

protected:
  explicit Wrapper(std::string profile) : profile(std::move(profile)) {
  }

  /**
   * LLM: @brief Validates routing results and identifies problematic locations.
   *
   * Analyzes unfound route counts to determine if any location has routing
   * issues. Identifies the location with the most routing failures and throws
   * an exception with detailed error information.
   *
   * @param locs Vector of all locations in the problem.
   * @param nb_unfound_from_loc Count of unfound routes originating from each
   * location.
   * @param nb_unfound_to_loc Count of unfound routes terminating at each
   * location.
   * @throws RoutingException if any location has unfound routes, with details
   * about the most problematic location.
   */
  static void check_unfound(const std::vector<Location>& locs,
                            const std::vector<unsigned>& nb_unfound_from_loc,
                            const std::vector<unsigned>& nb_unfound_to_loc) {
    assert(nb_unfound_from_loc.size() == nb_unfound_to_loc.size());
    unsigned max_unfound_routes_for_a_loc = 0;
    unsigned error_loc = 0; // Initial value never actually used.
    std::string error_direction;
    // Finding the "worst" location for unfound routes.
    for (unsigned i = 0; i < nb_unfound_from_loc.size(); ++i) {
      if (nb_unfound_from_loc[i] > max_unfound_routes_for_a_loc) {
        max_unfound_routes_for_a_loc = nb_unfound_from_loc[i];
        error_loc = i;
        error_direction = "from ";
      }
      if (nb_unfound_to_loc[i] > max_unfound_routes_for_a_loc) {
        max_unfound_routes_for_a_loc = nb_unfound_to_loc[i];
        error_loc = i;
        error_direction = "to ";
      }
    }
    if (max_unfound_routes_for_a_loc > 0) {
      std::string error_msg = "Unfound route(s) ";
      error_msg += error_direction;
      error_msg += std::format("location [{:.6f},{:.6f}]",
                               locs[error_loc].lon(),
                               locs[error_loc].lat());

      throw RoutingException(error_msg);
    }
  }
};

} // namespace vroom::routing

#endif
