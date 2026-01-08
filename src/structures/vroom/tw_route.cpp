/*

This file is part of VROOM.

Copyright (c) 2015-2025, Julien Coupey.
All rights reserved (see LICENSE).

*/

#include <algorithm>
#include <cassert>
#include <csignal>
#include <cstddef>
#include <string>

#include "structures/vroom/tw_route.h"
#include "utils/helpers.h"
#include "utils/log.h"



namespace vroom {

/** LLM: 
 * @brief Constructor for a route with time windows.
 * 
 * Initializes the route with a single vehicle and no jobs.
 * Computationally, it establishes the baseline validity of the empty route by:
 * 1. Setting up initial earliest and latest dates for breaks based on vehicle time windows.
 * 2. Calculating forward and backward load margins for breaks to ensure capacity constraints are met.
 * 3. Verifying consistency of break time windows and service times.
 */
TWRoute::TWRoute(const Input& input, Index v, unsigned amount_size)
  : RawRoute(input, v, amount_size),
    v_start(input.vehicles[v].tw.start),
    v_end(input.vehicles[v].tw.end),
    breaks_at_rank({static_cast<unsigned>(input.vehicles[v].breaks.size())}),
    breaks_counts({static_cast<unsigned>(input.vehicles[v].breaks.size())}),
    break_earliest(input.vehicles[v].breaks.size()),
    break_latest(input.vehicles[v].breaks.size()),
    fwd_smallest_breaks_load_margin(input.vehicles[v].breaks.size()),
    bwd_smallest_breaks_load_margin(input.vehicles[v].breaks.size()) {
  const std::string break_error =
    std::format("Inconsistent breaks for vehicle {}.", input.vehicles[v].id);

  const auto& breaks = input.vehicles[v].breaks;

  Duration previous_earliest = v_start;

  // Store smallest margin component-wise.
  Amount fwd_smallest_margin = utils::max_amount(amount_size);
  Amount bwd_smallest_margin = utils::max_amount(amount_size);

  // LLM:Initialize break earliest dates and forward load margins.
  // LLM:Iterate through breaks to set their earliest start times based on previous breaks
  // LLM:and vehicle start time. Also track the tightest load margin encountered so far.
  for (Index i = 0; i < breaks.size(); ++i) {
    const auto& b = breaks[i];
    const auto b_tw = std::ranges::find_if(b.tws, [&](const auto& tw) {
      return previous_earliest <= tw.end;
    });
    if (b_tw == b.tws.end()) {
      throw InputException(break_error);
    }

    break_earliest[i] = std::max(previous_earliest, b_tw->start);

    previous_earliest = break_earliest[i] + b.service;

    if (b.max_load.has_value()) {
      const auto& max_load = b.max_load.value();
      for (std::size_t a = 0; a < amount_size; ++a) {
        if (max_load[a] < fwd_smallest_margin[a]) {
          fwd_smallest_margin[a] = max_load[a];
        }
      }
    }
    fwd_smallest_breaks_load_margin[i] = fwd_smallest_margin;
  }

  Duration next_latest = v_end;
  // LLM:Initialize break latest dates and backward load margins.
  // LLM:Iterate backwards through breaks to set their latest start times based on subsequent breaks
  // LLM:and vehicle end time. Also track the tightest load margin from the end.
  for (Index r_i = 0; r_i < breaks.size(); ++r_i) {
    const Index i = breaks.size() - 1 - r_i;
    const auto& b = breaks[i];

    if (next_latest < b.service) {
      throw InputException(break_error);
    }
    next_latest -= b.service;

    const auto b_tw =
      std::find_if(b.tws.rbegin(), b.tws.rend(), [&](const auto& tw) {
        return tw.start <= next_latest;
      });
    if (b_tw == b.tws.rend()) {
      throw InputException(break_error);
    }

    break_latest[i] = std::min(next_latest, b_tw->end);

    next_latest = break_latest[i];

    if (break_latest[i] < break_earliest[i]) {
      throw InputException(break_error);
    }

    if (b.max_load.has_value()) {
      const auto& max_load = b.max_load.value();
      for (std::size_t a = 0; a < amount_size; ++a) {
        if (max_load[a] < bwd_smallest_margin[a]) {
          bwd_smallest_margin[a] = max_load[a];
        }
      }
    }
    bwd_smallest_breaks_load_margin[i] = bwd_smallest_margin;
  }
}

/** LLM: 
 * @brief Retrieves information about the step preceding a given rank in the route.
 * 
 * This helper calculates the state (earliest completion time, location) after the job
 * at `rank - 1`. If `rank` is 0, it considers the vehicle start location.
 * This is crucial for evaluating travel times and time window feasibility for an insertion at `rank`.
 */
PreviousInfo TWRoute::previous_info(const Input& input,
                                    const Index job_rank,
                                    const Index rank) const {
  const auto& v = input.vehicles[v_rank];
  const auto& j = input.jobs[job_rank];

  PreviousInfo previous(v_start, 0);
  // LLM:Determine the state (earliest completion time, location) of the previous step
  // LLM:in the route to calculate travel time to the current job.
  if (rank > 0) {
    const auto& previous_job = input.jobs[route[rank - 1]];
    previous.earliest_end = earliest[rank - 1] + action_time[rank - 1];
    previous.travel_time_from_prev = v.duration(previous_job.location_index(), j.location_index());
    previous.location_index = previous_job.location.index();
  } else {
    if (has_start) {
      previous.location_index = v.start.value().index();
      previous.travel_time_from_prev = v.duration(previous.location_index, j.location_index());
    }
  }

  return previous;
}

/** LLM: 
 * @brief Retrieves information about the step following a given rank in the route.
 * 
 * This helper calculates the state (latest start time) required for the job at `rank` (or whatever is at `rank`).
 * If `rank` is the end of the route, it considers the vehicle end location.
 * Used to determine the deadline for the current step to ensure subsequent steps remain feasible.
 */
NextInfo TWRoute::next_info(const Input& input,
                            const Index job_rank,
                            const Index rank) const {
  const auto& v = input.vehicles[v_rank];
  const auto& j = input.jobs[job_rank];

  NextInfo next(v_end, 0);
  // LLM:Determine the state (latest start time) of the next step in the route
  // LLM:to calculate travel time from the current job.
  if (rank == route.size()) {
    if (has_end) {
      next.travel_time_to = v.duration(j.location_index(), v.end.value().index());
    }
  } else {
    next.latest_start = latest[rank];
    next.travel_time_to = v.duration(j.location_index(), input.jobs[route[rank]].location_index());
  }

  return next;
}

/** LLM: 
 * @brief Propagates earliest start times forward from a specific rank.
 * 
 * Updates the `earliest` validity intervals for all subsequent jobs and breaks in the route.
 * It ensures that each step starts no earlier than allowed by:
 * 1. The completion of the previous step plus travel time.
 * 2. Its own time window start.
 * 3. Any intervening breaks.
 */
void TWRoute::fwd_update_earliest_from(const Input& input, Index rank) {
  const auto& v = input.vehicles[v_rank];

  Duration current_earliest = earliest[rank];
  bool handle_last_breaks = true;

  // LLM:Propagate earliest start times forward from the given rank.
  // LLM:This involves updating earliest times for subsequent jobs and any breaks
  // LLM:that occur between them.
  for (Index i = rank + 1; i < route.size(); ++i) {
    const auto& next_j = input.jobs[route[i]];
    Duration remaining_travel_time =
      v.duration(input.jobs[route[i - 1]].location_index(), next_j.location_index());
    Duration previous_action_time = action_time[i - 1];

    // Update earliest dates and margins for breaks.
    assert(breaks_at_rank[i] <= breaks_counts[i]);
    Index break_rank = breaks_counts[i] - breaks_at_rank[i];

    // LLM:Process breaks scheduled before the next job.
    // LLM:Adjust travel time if breaks consume some of the travel duration.
    for (Index r = 0; r < breaks_at_rank[i]; ++r, ++break_rank) {
      const auto& b = v.breaks[break_rank];

      current_earliest += previous_action_time;

      const auto b_tw = std::ranges::find_if(b.tws, [&](const auto& tw) {
        return current_earliest <= tw.end;
      });
      assert(b_tw != b.tws.end());

      if (current_earliest < b_tw->start) {
        if (const auto margin = b_tw->start - current_earliest;
            margin < remaining_travel_time) {
          remaining_travel_time -= margin;
        } else {
          remaining_travel_time = 0;
        }

        current_earliest = b_tw->start;
      }

      break_earliest[break_rank] = current_earliest;
      previous_action_time = v.breaks[break_rank].service;
    }

    // Back to the job after breaks.
    current_earliest += previous_action_time + remaining_travel_time;

    const auto j_tw = std::ranges::find_if(next_j.tws, [&](const auto& tw) {
      return current_earliest <= tw.end;
    });
    assert(j_tw != next_j.tws.end());

    current_earliest = std::max(current_earliest, j_tw->start);

    // Check consistency except for situation where latest date has
    // been reset to 0 to force backward propagation after this call
    // to fwd_update_earliest_from.
    assert(current_earliest <= latest[i] || (i == rank + 1 && latest[i] == 0));
    if (current_earliest == earliest[i]) {
      // There won't be any further update so stop earliest date
      // propagation.
      handle_last_breaks = false;
      break;
    }

    earliest[i] = current_earliest;
  }

  if (handle_last_breaks) {
    // Update earliest dates and margins for potential breaks right
    // before route end.
    const Index i = route.size();
    Duration remaining_travel_time =
      (v.has_end())
        ? v.duration(input.jobs[route[i - 1]].location_index(), v.end.value().index())
        : 0;

    Duration previous_action_time = action_time[i - 1];

    assert(breaks_at_rank[i] <= breaks_counts[i]);
    Index break_rank = breaks_counts[i] - breaks_at_rank[i];

    // LLM:Process any remaining breaks scheduled after the last job.
    for (Index r = 0; r < breaks_at_rank[i]; ++r, ++break_rank) {
      const auto& b = v.breaks[break_rank];
      current_earliest += previous_action_time;

      const auto b_tw = std::ranges::find_if(b.tws, [&](const auto& tw) {
        return current_earliest <= tw.end;
      });
      assert(b_tw != b.tws.end());

      if (current_earliest < b_tw->start) {
        if (const auto margin = b_tw->start - current_earliest;
            margin < remaining_travel_time) {
          remaining_travel_time -= margin;
        } else {
          remaining_travel_time = 0;
        }

        current_earliest = b_tw->start;
      }

      break_earliest[break_rank] = current_earliest;
      previous_action_time = v.breaks[break_rank].service;
    }

    earliest_end =
      current_earliest + previous_action_time + remaining_travel_time;
    assert(earliest_end <= v_end);
  }
}

/** LLM: 
 * @brief Propagates latest start times backward from a specific rank.
 * 
 * Updates the `latest` validity intervals for all preceding jobs and breaks up to the start of the route.
 * It ensures that each step starts no later than allowed to still meet:
 * 1. The start of the subsequent step minus travel time and service time.
 * 2. Its own time window end.
 * 3. Any intervening breaks.
 */
void TWRoute::bwd_update_latest_from(const Input& input, Index rank) {
  const auto& v = input.vehicles[v_rank];

  Duration current_latest = latest[rank];
  bool handle_first_breaks = true;

  // LLM:Propagate latest start times backward from the given rank.
  // LLM:This involves updating latest times for preceding jobs and any breaks
  // LLM:that occur between them.
  for (Index next_i = rank; next_i > 0; --next_i) {
    const auto& previous_j = input.jobs[route[next_i - 1]];
    Duration remaining_travel_time =
      v.duration(previous_j.location_index(), input.jobs[route[next_i]].location_index());

    // Update latest dates and margins for breaks.
    assert(breaks_at_rank[next_i] <= breaks_counts[next_i]);
    Index break_rank = breaks_counts[next_i];

    // LLM:Process breaks scheduled before the current job (in backward iteration).
    // LLM:Adjust travel time if breaks consume some of the travel duration.
    for (Index r = 0; r < breaks_at_rank[next_i]; ++r) {
      --break_rank;

      const auto& b = v.breaks[break_rank];
      assert(b.service <= current_latest);
      current_latest -= b.service;

      const auto b_tw =
        std::find_if(b.tws.rbegin(), b.tws.rend(), [&](const auto& tw) {
          return tw.start <= current_latest;
        });
      assert(b_tw != b.tws.rend());

      if (b_tw->end < current_latest) {
        if (const auto margin = current_latest - b_tw->end;
            margin < remaining_travel_time) {
          remaining_travel_time -= margin;
        } else {
          remaining_travel_time = 0;
        }

        current_latest = b_tw->end;
      }

      break_latest[break_rank] = current_latest;
    }

    // Back to the job after breaks.
    auto gap = action_time[next_i - 1] + remaining_travel_time;
    assert(gap <= current_latest);
    current_latest -= gap;

    const auto j_tw =
      std::find_if(previous_j.tws.rbegin(),
                   previous_j.tws.rend(),
                   [&](const auto& tw) { return tw.start <= current_latest; });
    assert(j_tw != previous_j.tws.rend());

    current_latest = std::min(current_latest, j_tw->end);

    assert(earliest[next_i - 1] <= current_latest);
    if (current_latest == latest[next_i - 1]) {
      // There won't be any further update so stop latest date
      // propagation.
      handle_first_breaks = false;
      break;
    }

    latest[next_i - 1] = current_latest;
  }

  if (handle_first_breaks) {
    // Update latest dates and margins for breaks right before the
    // first job.
    const Index next_i = 0;

    assert(breaks_at_rank[next_i] <= breaks_counts[next_i]);
    Index break_rank = breaks_counts[next_i];
    // LLM:Process any remaining breaks scheduled before the first job.
    for (Index r = 0; r < breaks_at_rank[next_i]; ++r) {
      --break_rank;
      const auto& b = v.breaks[break_rank];

      assert(b.service <= current_latest);
      current_latest -= b.service;

      const auto b_tw =
        std::find_if(b.tws.rbegin(), b.tws.rend(), [&](const auto& tw) {
          return tw.start <= current_latest;
        });
      assert(b_tw != b.tws.rend());
      if (b_tw->end < current_latest) {
        current_latest = b_tw->end;
      }

      break_latest[break_rank] = current_latest;
    }
  }
}

/** LLM: 
 * @brief Updates the latest valid start time for the last job in the route.
 * 
 * Specifically handles the constraint propagation from the vehicle's end time and any breaks
 * occurring after the last job. This anchors the backward propagation process.
 */
void TWRoute::update_last_latest_date(const Input& input) {
  assert(!route.empty());

  const auto& v = input.vehicles[v_rank];
  auto next = next_info(input, route.back(), route.size());

  // Latest date for breaks before end.
  Index break_rank = breaks_counts[route.size()];
  // LLM:Calculate the latest possible start times for breaks occurring after the last job
  // LLM:but before the route end.
  for (Index r = 0; r < breaks_at_rank[route.size()]; ++r) {
    --break_rank;
    const auto& b = v.breaks[break_rank];

    assert(b.service <= next.latest_start);
    next.latest_start -= b.service;

    const auto b_tw =
      std::find_if(b.tws.rbegin(), b.tws.rend(), [&](const auto& tw) {
        return tw.start <= next.latest_start;
      });
    assert(b_tw != b.tws.rend());

    if (b_tw->end < next.latest_start) {
      if (const auto margin = next.latest_start - b_tw->end; margin < next.travel_time_to) {
        next.travel_time_to -= margin;
      } else {
        next.travel_time_to = 0;
      }

      next.latest_start = b_tw->end;
    }

    break_latest[break_rank] = next.latest_start;
  }

  // Latest date for last job.
  const auto& j = input.jobs[route.back()];
  const auto gap = action_time.back() + next.travel_time_to;
  assert(gap <= next.latest_start);
  next.latest_start -= gap;

  const auto j_tw =
    std::find_if(j.tws.rbegin(), j.tws.rend(), [&](const auto& tw) {
      return tw.start <= next.latest_start;
    });
  assert(j_tw != j.tws.rend());

  latest.back() = std::min(next.latest_start, j_tw->end);
}

/** LLM: 
 * @brief Recalculates action times (setup + service) for jobs starting from a rank.
 * 
 * Setup time is context-dependent (depends on the previous location). This function ensures
 * that if a job's predecessor changes, the setup cost is correctly applied or removed.
 */
void TWRoute::fwd_update_action_time_from(const Input& input, Index rank) {
  Index current_index = input.jobs[route[rank]].location_index();

  // LLM:Update action times (setup + service) for jobs starting from the given rank.
  // LLM:Setup time is included only if the location changes from the previous job.
  for (Index i = rank + 1; i < route.size(); ++i) {
    const auto& next_j = input.jobs[route[i]];
    const auto next_index = next_j.location_index();

    const auto next_action_time =
      (next_index == current_index)
        ? next_j.services[v_type]
        : next_j.setups[v_type] + next_j.services[v_type];

    action_time[i] = next_action_time;
    current_index = next_index;
  }
}

/** LLM: 
 * @brief Updates forward cumulative load margins for breaks.
 * 
 * Re-evaluates how much additional load can be carried *before* violating the max load
 * constraint of any subsequent break. This creates a "safety margin" lookup for quick feasibility checks
 * during insertions.
 */
void TWRoute::fwd_update_breaks_load_margin_from(const Input& input,
                                                 Index rank) {
  const auto& v = input.vehicles[v_rank];

  // Last valid fwd_smallest value, if any.
  auto fwd_smallest =
    (breaks_counts[rank] == 0)
      ? utils::max_amount(input.get_amount_size())
      : fwd_smallest_breaks_load_margin[breaks_counts[rank] - 1];

  // LLM:Update forward load margins for breaks starting from the given rank.
  // LLM:This ensures that for each break, we know the minimum margin available
  // LLM:between the current load and the break's max load capacity up to that point.
  for (Index i = rank; i <= route.size(); ++i) {
    if (breaks_at_rank[i] != 0) {
      // Update for breaks right before job at rank i.
      const auto& current_load = load_at_step(i);

      for (auto break_rank = breaks_counts[i] - breaks_at_rank[i];
           break_rank < breaks_counts[i];
           ++break_rank) {
        const auto& b = v.breaks[break_rank];

        assert(b.is_valid_for_load(current_load));
        auto current_margin = (b.max_load.has_value())
                                ? b.max_load.value() - current_load
                                : utils::max_amount(input.get_amount_size());

        for (std::size_t a = 0; a < fwd_smallest.size(); ++a) {
          fwd_smallest[a] = std::min(fwd_smallest[a], current_margin[a]);
        }

        assert(input.zero_amount() <= fwd_smallest);
        fwd_smallest_breaks_load_margin[break_rank] = fwd_smallest;
      }
    }
  }
}

/** LLM: 
 * @brief Updates backward cumulative load margins for breaks.
 * 
 * Similar to the forward update, but evaluates margins from the end of the route backwards.
 * Used to quickly check if picking up load at a certain point will violate a later break's capacity.
 */
void TWRoute::bwd_update_breaks_load_margin_from(const Input& input,
                                                 Index rank) {
  const auto& v = input.vehicles[v_rank];

  // Last valid bwd_smallest value, if any.
  auto bwd_smallest = (breaks_counts[rank] == breaks_counts.back())
                        ? utils::max_amount(input.get_amount_size())
                        : bwd_smallest_breaks_load_margin[breaks_counts[rank]];

  // LLM:Update backward load margins for breaks starting from the given rank and going backwards.
  // LLM:This ensures that for each break, we know the minimum margin available
  // LLM:between the current load and the break's max load capacity from that point onwards.
  for (Index bwd_i = 0; bwd_i <= rank; ++bwd_i) {
    const auto i = rank - bwd_i;
    if (breaks_at_rank[i] != 0) {
      // Update for breaks right before job at rank i.
      const auto& current_load = load_at_step(i);

      for (unsigned bwd_break_count = 0; bwd_break_count < breaks_at_rank[i];
           ++bwd_break_count) {
        const auto break_rank = breaks_counts[i] - 1 - bwd_break_count;
        const auto& b = v.breaks[break_rank];

        assert(b.is_valid_for_load(current_load));
        auto current_margin = (b.max_load.has_value())
                                ? b.max_load.value() - current_load
                                : utils::max_amount(input.get_amount_size());

        for (std::size_t a = 0; a < bwd_smallest.size(); ++a) {
          bwd_smallest[a] = std::min(bwd_smallest[a], current_margin[a]);
        }

        assert(input.zero_amount() <= bwd_smallest);
        bwd_smallest_breaks_load_margin[break_rank] = bwd_smallest;
      }
    }
  }
}

/** LLM: 
 * @brief Constructor for the OrderChoice helper.
 * 
 * Initializes the choice context by finding the first feasible time windows for both
 * the job and the break relative to the previous step's completion.
 */
OrderChoice::OrderChoice(const Input& input,
                         const Index job_rank,
                         const Break& b,
                         const PreviousInfo& previous)
  : input(input),
    // LLM:Find the first valid time window for the job that allows arrival after the previous step.
    j_tw(std::ranges::find_if(input.jobs[job_rank].tws,
                              [&](const auto& tw) {
                                return previous.earliest_end + previous.travel_time_from_prev <=
                                       tw.end;
                              })),
    // LLM:Find the first valid time window for the break that allows arrival after the previous step.
    b_tw(std::ranges::find_if(b.tws, [&](const auto& tw) {
      return previous.earliest_end <= tw.end;
    })) {
}

/** LLM: 
 * @brief Determines the optimal order for a job and a break that need to happen in the same interval.
 * 
 * When a break falls between two jobs (or start/end), and we insert a new job there, we must decide
 * whether to schedule: `Previous -> Job -> Break -> Next` or `Previous -> Break -> Job -> Next`.
 * This function evaluates time windows and load constraints to determine which orderings are valid
 * and prefers the one that minimizes delays or fits specific job types (pickup/delivery).
 */
OrderChoice TWRoute::order_choice(const Input& input,
                                  const Index job_rank,
                                  const Duration job_action_time,
                                  const Break& b,
                                  const PreviousInfo& previous,
                                  const NextInfo& next,
                                  const Amount& current_load,
                                  bool check_max_load) const {
  OrderChoice oc(input, job_rank, b, previous);
  const auto& v = input.vehicles[v_rank];
  const auto& j = input.jobs[job_rank];

  if (oc.j_tw == j.tws.end() || oc.b_tw == b.tws.end()) {
    // If either job or break can't fit first, then none of the
    // orderings are valid.
    return oc;
  }

  Duration job_then_break_end;
  Duration break_then_job_end;

  // Try putting job first then break.
  // LLM:Calculate the earliest end time if the job is performed before the break.
  const Duration earliest_job_end =
    std::max(previous.earliest_end + previous.travel_time_from_prev, oc.j_tw->start) +
    job_action_time;
  Duration job_then_break_margin = 0;

  const auto new_b_tw = std::ranges::find_if(b.tws, [&](const auto& tw) {
    return earliest_job_end <= tw.end;
  });
  if (new_b_tw == b.tws.end()) {
    // Break does not fit after job due to its time windows. Only
    // option is to choose break first, if valid for max_load.
    oc.add_break_first = !check_max_load || b.is_valid_for_load(current_load);
    return oc;
  }

  Duration travel_after_break = next.travel_time_to;
  // LLM:Adjust travel time after the break if there is a waiting time (margin)
  // LLM:between the job completion and the break start.
  if (earliest_job_end < new_b_tw->start) {
    job_then_break_margin = new_b_tw->start - earliest_job_end;
    if (job_then_break_margin < travel_after_break) {
      travel_after_break -= job_then_break_margin;
    } else {
      travel_after_break = 0;
    }

    job_then_break_end = oc.b_tw->start + b.service;
  } else {
    job_then_break_end = earliest_job_end + b.service;
  }

  if (job_then_break_end + travel_after_break > next.latest_start) {
    // Starting the break is possible but then next step is not.
    oc.add_break_first = true;
    return oc;
  }

  if (check_max_load && j.type == JOB_TYPE::SINGLE &&
      (!b.is_valid_for_load(current_load + j.pickup - j.delivery) ||
       !(j.pickup <= bwd_smallest_breaks_load_margin[v.break_rank(b.id)]))) {
    // Break won't fit right after job for load reason.
    oc.add_break_first = b.is_valid_for_load(current_load);
    return oc;
  }

  // Try putting break first then job.
  // LLM:Check if putting the break first is valid with respect to load constraints.
  if (check_max_load && !b.is_valid_for_load(current_load)) {
    // Not doable based on max_load, only option is to choose job
    // first.
    oc.add_job_first = true;
    return oc;
  }

  travel_after_break = previous.travel_time_from_prev;
  Duration earliest_job_start = previous.earliest_end;

  // LLM:Calculate the earliest start time for the job if the break is performed first.
  // LLM:Adjust travel time if there is a waiting time before the break.
  if (previous.earliest_end < oc.b_tw->start) {
    if (const auto margin = oc.b_tw->start - previous.earliest_end;
        margin < travel_after_break) {
      travel_after_break -= margin;
    } else {
      travel_after_break = 0;
    }

    earliest_job_start = oc.b_tw->start;
  }

  earliest_job_start += b.service + travel_after_break;

  const auto new_j_tw = std::ranges::find_if(j.tws, [&](const auto& tw) {
    return earliest_job_start <= tw.end;
  });

  if (new_j_tw == j.tws.end()) {
    // Job does not fit after break due to its time windows. Only
    // option is to choose job first.
    oc.add_job_first = true;
    return oc;
  }
  break_then_job_end =
    std::max(earliest_job_start, new_j_tw->start) + job_action_time;

  if (break_then_job_end + next.travel_time_to > next.latest_start) {
    // Arrival at the job is valid but next step is not.
    oc.add_job_first = true;
    return oc;
  }

  // Now both ordering options are doable based on timing constraints.

  // For a pickup, we favor putting the pickup first, except if adding
  // the delivery afterwards is not possible. This is mandatory to
  // avoid heuristically forcing a pickup -> break choice resulting in
  // invalid options, while break -> pickup -> delivery might be
  // valid.
  if (j.type == JOB_TYPE::PICKUP) {
    const auto& matching_d = input.jobs[job_rank + 1];
    assert(matching_d.type == JOB_TYPE::DELIVERY);

    // Try pickup -> break -> delivery.
    auto delivery_travel = v.duration(j.location_index(), matching_d.location_index());
    if (job_then_break_margin < delivery_travel) {
      delivery_travel -= job_then_break_margin;
    } else {
      delivery_travel = 0;
    }
    const Duration pb_d_candidate = job_then_break_end + delivery_travel;
    if (const auto pb_d_tw = std::ranges::find_if(matching_d.tws,
                                                  [&](const auto& tw) {
                                                    return pb_d_candidate <=
                                                           tw.end;
                                                  });
        pb_d_tw != matching_d.tws.end() &&
        (!check_max_load || b.is_valid_for_load(current_load + j.pickup))) {
      // pickup -> break -> delivery is doable, choose pickup first.
      oc.add_job_first = true;
      return oc;
    }

    // Previous order not doable, so try pickup -> delivery -> break.
    const Duration delivery_candidate =
      earliest_job_end + v.duration(j.location_index(), matching_d.location_index());
    if (const auto d_tw = std::ranges::find_if(matching_d.tws,
                                               [&](const auto& tw) {
                                                 return delivery_candidate <=
                                                        tw.end;
                                               });
        d_tw != matching_d.tws.end()) {
      const auto matching_d_action_time =
        (matching_d.location_index() == j.location_index())
          ? matching_d.services[v_type]
          : matching_d.setups[v_type] + matching_d.services[v_type];

      const Duration break_candidate =
        std::max(delivery_candidate, d_tw->start) + matching_d_action_time;

      const auto after_d_b_tw =
        std::ranges::find_if(b.tws, [&](const auto& tw) {
          return break_candidate <= tw.end;
        });
      if (after_d_b_tw != b.tws.end()) {
        // pickup -> delivery -> break is doable, choose pickup first.
        assert(!check_max_load || b.is_valid_for_load(current_load));
        oc.add_job_first = true;
        return oc;
      }
    }

    // Doing pickup first actually leads to infeasible options, so put
    // break first.
    oc.add_break_first = true;
    return oc;
  }

  // For a single job, we pick the ordering minimizing earliest end
  // date for sequence.
  if (break_then_job_end < job_then_break_end) {
    oc.add_break_first = true;
  } else if (break_then_job_end == job_then_break_end) {
    // If end date is the same for both ordering options, decide based
    // on earliest deadline, except for deliveries. If a delivery
    // without TW constraint is postponed, it can introduce arbitrary
    // waiting time between zero max_load breaks.
    if (j.type == JOB_TYPE::DELIVERY || oc.j_tw->end <= oc.b_tw->end) {
      oc.add_job_first = true;
    } else {
      oc.add_break_first = true;
    }
  } else {
    oc.add_job_first = true;
  }

  return oc;
}


/**
 BRUNO: this predicates check that the insertion does not cause a return to depot with undelivered jobs

 this is a quick implementation, not optimized at all
 **/
bool TWRoute::is_no_return_to_depot_with_undelivered_jobs(const Input& input,                                                  
                                                  const Index first_rank
                                                  ) const {


  TRACE_LOG(" is_no_return_to_depot_with_undelivered_jobs()");

  // scan the route for deliveries after first_rank, 
  // and checks that their matching pickups are not before first_rank
  for (size_t r = first_rank; r < route.size(); r++) {


        TTRACE_LOG(" is_no_return_to_depot_with_undelivered_jobs[" << r << "]:" << route[r]
                              << " earliest:" << t2str(earliest[r])
                              << " latest:" << t2str(latest[r])
                              << " action_time:" << t2str(action_time[r]));



          const auto jobId = route[r];
          const auto& j = input.jobs[jobId];
          if (j.type == JOB_TYPE::DELIVERY) {
            size_t s;

            // now identify the matching pickup
            for (s = r - 1; s >= 0; s--) {
              const auto prevJobId = route[s];
              const auto& pj = input.jobs[prevJobId];
              if (pj.type == JOB_TYPE::PICKUP) {
                // BRUNO: is this the matching pickup?  i am not sure...
                const auto pd_match = pj.id + 1 == j.id;
                if (pd_match) {
                  // matching pickup found
                  if (s < first_rank) {
                    return false;
                  }
                  break;
                }
              

                // nice code generated by LLM: it check against the max delivery
                // duration const auto travelTime =
                // v.duration(pj.location_index(), j.location_index()); const auto
                // earliestStart = earliest[s] + action_time[s] + travelTime; if
                // (earliestStart > latest[r]){
                //   TTRACE_LOG(" Insertion breaks delivery TW for job "<< j.id<<"
                //   earliestStart:"<<t2str(earliestStart)<<"
                //   latest:"<<t2str(latest[r]) ); return false;
              }
              // if we hit s=0 at this point, it means no matching pickup found
              assert(s!=0);  
            }
            
          }
        }
  return true;
} 

/** LLM: 
 * @brief Checks if adding a sequence of jobs is feasible regarding time windows and breaks.
 * 
 * Simulates the insertion of a range of jobs into the route at a specific rank.
 * It verifies:
 * 1. Time window feasibility for all new jobs and displaced breaks.
 * 2. Load constraints (optional).
 * 3. Whether the insertion disrupts the feasibility of subsequent steps in the route.
 * 
 * This is a "dry run" check that does not modify the route.
 */
template <std::forward_iterator Iter>
bool TWRoute::is_valid_addition_for_tw(const Input& input,
                                       const Amount& delivery,
                                       const Iter first_job,
                                       const Iter last_job,
                                       const Index first_rank,
                                       const Index last_rank,
                                       bool check_max_load) const {
  assert(first_job <= last_job);
  assert(first_rank <= last_rank);

  const auto& v = input.vehicles[v_rank];

  // Override this value if vehicle does not need this check anyway to
  // spare some work.
  check_max_load = v.has_break_max_load && check_max_load;
  // BRUNO: 1) initialize: current and next info
  // BRUNO: PreviousInfo and NextInfo structs hold state information: travel time +
  // earliest end date for previous step and latest start date for next step.
  PreviousInfo current(0, 0);
  // BRUNO: note that lastJobAtLastRank (renamed from next) is never modified past initialization
  NextInfo lastJobAtLastRank(0, 0);

  // Value initialization differ whether there are actually jobs added
  // or not.
  // LLM: Initialize the state (earliest start, location) for the insertion point
  // LLM: and update the state (latest start, travel) for the subsequent step.
  // LLM: This handles both insertion of new jobs and removal of existing jobs (when first_job == last_job).
  if (first_job < last_job) {
    current = previous_info(input, *first_job, first_rank);
    lastJobAtLastRank = next_info(input, *(last_job - 1), last_rank);
  } else {
    // This is actually a removal as no jobs are inserted.
    current.earliest_end = v_start;
    lastJobAtLastRank.latest_start = v_end;

    if (first_rank > 0) {
      const auto& previous_job = input.jobs[route[first_rank - 1]];
      current.earliest_end = earliest[first_rank - 1] + action_time[first_rank - 1];
      current.location_index = previous_job.location_index();

      if (last_rank < route.size()) {
        lastJobAtLastRank.latest_start = latest[last_rank];
        lastJobAtLastRank.travel_time_to = v.duration(previous_job.location_index(),
                                 input.jobs[route[last_rank]].location_index());
      } else {
        if (has_end) {
          lastJobAtLastRank.travel_time_to = v.duration(previous_job.location_index(), v.end.value().index());
        }
      }
    } else {
      // BRUNO : first_rank==0
      if (last_rank < route.size()) {
        lastJobAtLastRank.latest_start = latest[last_rank];
        if (has_start) {
          current.location_index = v.start.value().index();
          lastJobAtLastRank.travel_time_to = v.duration(v.start.value().index(),
                                   input.jobs[route[last_rank]].location_index());
        }
      } else {
        // Emptying the whole route is valid.
        return true;
      }
    }
  }

  // Determine break range between first_rank and last_rank.
  Index current_break = breaks_counts[first_rank] - breaks_at_rank[first_rank];
  const Index last_break = breaks_counts[last_rank];

  // Maintain current load while adding insertion range. Initial load
  // is lowered based on removed range.
  Amount current_load;

  // LLM:Check if the initial load at the insertion point is valid with respect to
  // LLM:break max load constraints, considering the delivery amount of the new jobs.
  if (check_max_load) {
    const auto previous_init_load =
      (route.empty()) ? input.zero_amount() : load_at_step(first_rank);
    assert(delivery_in_range(first_rank, last_rank) <= previous_init_load);
    const Amount delta_delivery =
      delivery - delivery_in_range(first_rank, last_rank);

    if (current_break != 0 &&
        !(delta_delivery <=
          fwd_smallest_breaks_load_margin[current_break - 1])) {
      return false;
    }

    current_load = previous_init_load + delta_delivery;
  }
  // BRUNO: 2)  main loop - it has 3 parts:insert only next break, only next job, or consider both 
  // Propagate earliest dates for all jobs and breaks in their
  // respective addition ranges.
  auto current_job = first_job;
  // LLM:Iterate through the breaks that need to be scheduled.check for time window and load validity.
  while (current_job != last_job || current_break != last_break) {
    TTRACE_LOG("@"<< t2str(current.earliest_end)<<" current_job:#"<<*current_job);
    // BRUNO: 2.1):No more jobs to insert, only breaks remain to be scheduled.
    if (current_job == last_job) {

      // Compute earliest end date for break after last inserted jobs.
      const auto& b = v.breaks[current_break];

      const auto b_tw = std::ranges::find_if(b.tws, [&](const auto& tw) {
        return current.earliest_end <= tw.end;
      });

      if (b_tw == b.tws.end()) {
        // Break does not fit due to its time windows.
        return false;
      }

      if (check_max_load && !b.is_valid_for_load(current_load)) {
        // Break does not fit due to current load.
        return false;
      }

      if (current.earliest_end < b_tw->start) {
        if (const auto margin = b_tw->start - current.earliest_end;
            margin < lastJobAtLastRank.travel_time_to) {
          lastJobAtLastRank.travel_time_to -= margin;
        } else {
          lastJobAtLastRank.travel_time_to = 0;
        }

        current.earliest_end = b_tw->start;
      }

      current.earliest_end += b.service;

      ++current_break;
      continue;
    }    
    // We still have jobs to go through.
    const auto& j = input.jobs[*current_job];

     // BRUNO: 2.2):No more breaks to insert, only jobs remain to be scheduled.
    if (current_break == last_break) {     
      TTRACE_LOG("@"<< t2str(current.earliest_end)<<" travel to job  #"<< j.id<<" driving:"<< t2str(current.travel_time_from_prev)); 
      // BRUNO: note that 'current.earliest_end' is used as a variable and
      // will corrspond to other time valued during this block
      // Compute earliest end date for job after last inserted breaks.
      current.earliest_end += current.travel_time_from_prev;
      TTRACE_LOG("@"<< t2str(current.earliest_end)<<" arrived  ");
      // BRUNO: find 1st valid time window 
      const auto j_tw = std::ranges::find_if(j.tws, [&](const auto& tw) {
        return current.earliest_end <= tw.end;
      });
      if (j_tw == j.tws.end()) {
        return false;
      }

      // BRUNO: update earliest date with job action time
      const auto job_action_time = (j.location_index() == current.location_index)
                                     ? j.services[v_type]
                                     : j.setups[v_type] + j.services[v_type];
      TTRACE_LOG("    job action time: "<<t2str(job_action_time));
      current.location_index = j.location_index();
      TTRACE_LOG("    tw "<<t2str(j_tw->start)<<"-"<< t2str(j_tw->end)<<" , waiting for tw: "<<t2str(j_tw->start<current.earliest_end?0:j_tw->start-current.earliest_end));
      current.earliest_end =
        std::max(current.earliest_end, j_tw->start) + job_action_time;
      TTRACE_LOG("@"<< t2str( current.earliest_end)<<" job "<< j.id<<" done");
      if (check_max_load) {
        assert(j.delivery <= current_load);
        current_load += (j.pickup - j.delivery);
      }



     
      if (is_no_return_to_depot_with_undelivered_jobs(input, first_rank)){
        return false;
      }
     

      ++current_job;
      if (current_job != last_job) {
        // Record for travel time to next current job.
        current.travel_time_from_prev =
          v.duration(j.location_index(), input.jobs[*current_job].location_index());
      }

     
  
      continue;
    }
    // BRUNO: 2.3)
    
    // We still have both jobs and breaks to go through, so decide on
    // ordering.
    const auto& b = v.breaks[current_break];
    const auto job_action_time = (j.location_index() == current.location_index)
                                   ? j.services[v_type]
                                   : j.setups[v_type] + j.services[v_type];

    auto oc = order_choice(input,
                           *current_job,
                           job_action_time,
                           b,
                           current,
                           lastJobAtLastRank,
                           current_load,
                           check_max_load);

    if (!oc.add_job_first && !oc.add_break_first) {
      // Infeasible insertion.
      return false;
    }

    // Feasible insertion based on time windows, now update next end
    // time with given insertion choice.
    assert(oc.add_job_first xor oc.add_break_first);
    if (oc.add_break_first) {
      if (check_max_load && !b.is_valid_for_load(current_load)) {
        return false;
      }

      if (current.earliest_end < oc.b_tw->start) {
        if (const auto margin = oc.b_tw->start - current.earliest_end;
            margin < current.travel_time_from_prev) {
          current.travel_time_from_prev -= margin;
        } else {
          current.travel_time_from_prev = 0;
        }

        current.earliest_end = oc.b_tw->start;
      }

      current.earliest_end += b.service;

      ++current_break;
    }
    if (oc.add_job_first) {
      current.location_index = j.location_index();

      current.earliest_end =
        std::max(current.earliest_end + current.travel_time_from_prev, oc.j_tw->start) +
        job_action_time;

      if (check_max_load) {
        assert(j.delivery <= current_load);
        current_load += (j.pickup - j.delivery);
      }

      ++current_job;
      if (current_job != last_job) {
        // Account for travel time to next current job.
        current.travel_time_from_prev =
          v.duration(j.location_index(), input.jobs[*current_job].location_index());
      }
    }
  }

  if (check_max_load && last_break < v.breaks.size()) {
    const auto previous_final_load =
      (route.empty()) ? input.zero_amount() : load_at_step(last_rank);

    const Amount delta_pickup = current_load - previous_final_load;

    if (!(delta_pickup <= bwd_smallest_breaks_load_margin[last_break])) {
      return false;
    }
  }
  // BRUNO: 3) maybe update action time due to setup for the first job after replace range
  // BRUNO: NOTE: NextInfo.latest_start is computed based on route.latest[], which takes into account
  // subsequent jobs time constraints, so we do not need
  // to check all subsequent jobs, only the first one after the replaced range.

  if (last_rank < route.size() &&
      input.jobs[route[last_rank]].location_index() != current.location_index) {
    // There is a task right after replace range and setup time does
    // apply to it.
    const auto& j_after = input.jobs[route[last_rank]];
    auto new_action_time = j_after.setups[v_type] + j_after.services[v_type];
    if (action_time[last_rank] < new_action_time) {
      // Setup time did not previously apply to that task as action
      // time has increased. In that case the margin check for job at
      // last_rank may be OK in the return clause below, BUT shifting
      // earliest date for next task with new setup time may make it
      // not doable anymore.
      auto earliest_after = current.earliest_end + lastJobAtLastRank.travel_time_to;
      const auto j_after_tw =
        std::ranges::find_if(j_after.tws, [&](const auto& tw) {
          return earliest_after <= tw.end;
        });
      if (j_after_tw == j_after.tws.end()) {
        return false;
      }
      earliest_after = std::max(earliest_after, j_after_tw->start);

      auto next_after = next_info(input, route[last_rank], last_rank + 1);

      // Go through breaks right after.
      Index break_rank =
        breaks_counts[last_rank + 1] - breaks_at_rank[last_rank + 1];

      for (Index r = 0; r < breaks_at_rank[last_rank + 1]; ++r, ++break_rank) {
        const auto& b = v.breaks[break_rank];

        earliest_after += new_action_time;

        const auto b_tw = std::ranges::find_if(b.tws, [&](const auto& tw) {
          return earliest_after <= tw.end;
        });
        if (b_tw == b.tws.end()) {
          // Break does not fit due to its time windows.
          return false;
        }

        if (earliest_after < b_tw->start) {
          if (const auto margin = b_tw->start - earliest_after;
              margin < next_after.travel_time_to) {
            next_after.travel_time_to -= margin;
          } else {
            next_after.travel_time_to = 0;
          }

          earliest_after = b_tw->start;
        }

        new_action_time = v.breaks[break_rank].service;
      }

      if (earliest_after + new_action_time + next_after.travel_time_to >
          next_after.latest_start) {
        return false;
      }
    }
  }

  return current.earliest_end + lastJobAtLastRank.travel_time_to <= lastJobAtLastRank.latest_start;
}

/** LLM: 
 * @brief Modifies the route by replacing a range of jobs with a new sequence.
 * 
 * This is the core mutation function. It handles:
 * 1. Removing a range of existing jobs (can be empty for simple insertion).
 * 2. Inserting a new range of jobs (can be empty for deletion).
 * 3. Updating route structures: `route`, `earliest`, `latest`, `action_time`.
 * 4. Re-calculating break positions and validity.
 * 5. Propagating time window and load updates forward and backward to maintain route consistency.
 */
template <std::random_access_iterator Iter>
void TWRoute::replace(const Input& input,
                      const Amount& delivery,
                      const Iter first_job,
                      const Iter last_job,
                      const Index first_rank,
                      const Index last_rank) {
  
  assert(first_job <= last_job);
  assert(first_rank <= last_rank);
  TRACE_LOG("replace() "<< first_rank << " to " << last_rank << " with "
            << std::distance(first_job, last_job) << " jobs");
  const auto& v = input.vehicles[v_rank];

  PreviousInfo current(0, 0);
  NextInfo next(0, 0);

  // Value initialization differ whether there are actually jobs added
  // or not.
  // LLM:Initialize the state (earliest start, location) for the insertion point
  // LLM:and the state (latest start, travel) for the subsequent step.
  // LLM:This handles both insertion of new jobs and removal of existing jobs (when first_job == last_job).
  if (first_job < last_job) {
    current = previous_info(input, *first_job, first_rank);
    next = next_info(input, *(last_job - 1), last_rank);
  } else {
    // This is actually a removal as no jobs are inserted.
    current.earliest_end = v_start;
    next.latest_start = v_end;

    if (first_rank > 0) {
      const auto& previous_job = input.jobs[route[first_rank - 1]];
      const auto previous_index = previous_job.location_index();
      current.earliest_end = earliest[first_rank - 1] + action_time[first_rank - 1];
      current.location_index = previous_index;

      if (last_rank < route.size()) {
        next.latest_start = latest[last_rank];
        next.travel_time_to =
          v.duration(previous_index, input.jobs[route[last_rank]].location_index());
      } else {
        if (has_end) {
          next.travel_time_to = v.duration(previous_index, v.end.value().index());
        }
      }
    } else {
      if (last_rank < route.size()) {
        next.latest_start = latest[last_rank];
        if (has_start) {
          current.location_index = v.start.value().index();
          next.travel_time_to = v.duration(v.start.value().index(),
                                   input.jobs[route[last_rank]].location_index());
        }
      }
    }
  }

  // Determine break range between first_rank and last_rank.
  Index current_break = breaks_counts[first_rank] - breaks_at_rank[first_rank];
  const Index last_break = breaks_counts[last_rank];

  // Maintain current load while adding insertion range. Initial load
  // is lowered based on removed range.
  const auto previous_init_load =
    (route.empty()) ? input.zero_amount() : load_at_step(first_rank);
  const auto previous_final_load =
    (route.empty()) ? input.zero_amount() : load_at_step(last_rank);
  assert(delivery_in_range(first_rank, last_rank) <= previous_init_load);
  const Amount delta_delivery =
    delivery - delivery_in_range(first_rank, last_rank);
  Amount current_load = previous_init_load + delta_delivery;

  // Update all break load margins prior to modified range.
  assert(current_break == 0 ||
         delta_delivery <= fwd_smallest_breaks_load_margin[current_break - 1]);
  // LLM:Update forward load margins for breaks occurring before the modified range.
  // LLM:This is necessary because the delivery amount change affects the load profile
  // LLM:for all preceding breaks.
  for (std::size_t i = 0; i < current_break; ++i) {
    assert(delta_delivery <= fwd_smallest_breaks_load_margin[i]);

    // Manually decrement margin to avoid overflows that would end up
    // in a negative margin with a plain
    // fwd_smallest_breaks_load_margin[i] -= delta_delivery;
    for (std::size_t a = 0; a < delta_delivery.size(); ++a) {
      if ((-delta_delivery[a]) <= (std::numeric_limits<Capacity>::max() -
                                   fwd_smallest_breaks_load_margin[i][a])) {
        fwd_smallest_breaks_load_margin[i][a] -= delta_delivery[a];
      } else {
        fwd_smallest_breaks_load_margin[i][a] =
          std::numeric_limits<Capacity>::max();
      }
    }
  }

  unsigned previous_breaks_counts =
    (first_rank != 0) ? breaks_counts[first_rank - 1] : 0;

  // Adjust various vector sizes. Dummy inserted values and unmodified
  // old values in the insertion range will be overwritten below.
  const unsigned erase_count = last_rank - first_rank;
  const unsigned add_count = std::distance(first_job, last_job);

  // Update data structures. For earliest and latest dates, we need to
  // overwrite old values. Otherwise they may happen to be identical
  // to new computed values and stop propagation inside
  // fwd_update_earliest_from and bwd_update_latest_from below.
  // LLM:Resize and update the route and associated data vectors (earliest, latest, action_time, etc.)
  // LLM:to accommodate the new jobs or removal of jobs.
  if (add_count < erase_count) {
    auto to_erase = erase_count - add_count;
    route.erase(route.begin() + first_rank,
                route.begin() + first_rank + to_erase);
    earliest.erase(earliest.begin() + first_rank,
                   earliest.begin() + first_rank + to_erase);
    latest.erase(latest.begin() + first_rank,
                 latest.begin() + first_rank + to_erase);
    action_time.erase(action_time.begin() + first_rank,
                      action_time.begin() + first_rank + to_erase);
    breaks_at_rank.erase(breaks_at_rank.begin() + first_rank,
                         breaks_at_rank.begin() + first_rank + to_erase);
    breaks_counts.erase(breaks_counts.begin() + first_rank,
                        breaks_counts.begin() + first_rank + to_erase);

    std::fill(earliest.begin() + first_rank,
              earliest.begin() + first_rank + add_count,
              std::numeric_limits<Duration>::max());
    std::fill(latest.begin() + first_rank,
              latest.begin() + first_rank + add_count,
              0);
  } else {
    std::fill(earliest.begin() + first_rank,
              earliest.begin() + first_rank + erase_count,
              std::numeric_limits<Duration>::max());
    std::fill(latest.begin() + first_rank,
              latest.begin() + first_rank + erase_count,
              0);

    auto to_insert = add_count - erase_count;
    route.insert(route.begin() + first_rank, to_insert, 0);
    earliest.insert(earliest.begin() + first_rank, to_insert, 0);
    latest.insert(latest.begin() + first_rank, to_insert, 0);
    action_time.insert(action_time.begin() + first_rank, to_insert, 0);
    breaks_at_rank.insert(breaks_at_rank.begin() + first_rank, to_insert, 0);
    breaks_counts.insert(breaks_counts.begin() + first_rank, to_insert, 0);
  }

  // Current rank in route/earliest/latest/action_time vectors.
  Index current_job_rank = first_rank;
  unsigned breaks_before = 0;

  // Propagate earliest dates (and action times) for all jobs and
  // breaks in their respective addition ranges.
  auto current_job = first_job;
  // LLM:Iterate through the new jobs and any breaks that need to be scheduled.
  // LLM:Determine the order of jobs and breaks, update their earliest start times,
  // LLM:and update the route structure with the chosen order.
  while (current_job != last_job || current_break != last_break) {
    if (current_job == last_job) {
      // Compute earliest end date for break after last inserted jobs.
      const auto& b = v.breaks[current_break];
      assert(b.is_valid_for_load(current_load));

      const auto b_tw = std::ranges::find_if(b.tws, [&](const auto& tw) {
        return current.earliest_end <= tw.end;
      });
      assert(b_tw != b.tws.end());

      if (current.earliest_end < b_tw->start) {
        if (const auto margin = b_tw->start - current.earliest_end;
            margin < next.travel_time_to) {
          next.travel_time_to -= margin;
        } else {
          next.travel_time_to = 0;
        }

        current.earliest_end = b_tw->start;
      }
      break_earliest[current_break] = current.earliest_end;

      current.earliest_end += b.service;

      // Update break max load margin.
      auto current_margin = (b.max_load.has_value())
                              ? b.max_load.value() - current_load
                              : utils::max_amount(input.get_amount_size());
      if (current_break == 0) {
        // New fwd_smallest_breaks_load_margin is solely based on this
        // break max_load.
        fwd_smallest_breaks_load_margin[current_break] = current_margin;
      } else {
        const auto& previous_margin =
          fwd_smallest_breaks_load_margin[current_break - 1];
        for (std::size_t i = 0; i < previous_margin.size(); ++i) {
          fwd_smallest_breaks_load_margin[current_break][i] =
            std::min(previous_margin[i], current_margin[i]);
        }
      }

      ++breaks_before;
      ++current_break;
      continue;
    }

    // We still have jobs to go through.
    const auto& j = input.jobs[*current_job];

    if (current_break == last_break) {
      // Compute earliest end date for job after last inserted breaks.
      current.earliest_end += current.travel_time_from_prev;

      const auto j_tw = std::ranges::find_if(j.tws, [&](const auto& tw) {
        return current.earliest_end <= tw.end;
      });
      assert(j_tw != j.tws.end());

      current.earliest_end = std::max(current.earliest_end, j_tw->start);

      route[current_job_rank] = *current_job;
      earliest[current_job_rank] = current.earliest_end;
      breaks_at_rank[current_job_rank] = breaks_before;
      breaks_counts[current_job_rank] = previous_breaks_counts + breaks_before;

      action_time[current_job_rank] = (j.location_index() == current.location_index)
                                        ? j.services[v_type]
                                        : j.setups[v_type] + j.services[v_type];
      current.location_index = j.location_index();
      current.earliest_end += action_time[current_job_rank];

      ++current_job_rank;
      previous_breaks_counts += breaks_before;
      breaks_before = 0;

      assert(j.delivery <= current_load);
      current_load += (j.pickup - j.delivery);

      ++current_job;
      if (current_job != last_job) {
        // Account for travel time to next current job.
        current.travel_time_from_prev =
          v.duration(j.location_index(), input.jobs[*current_job].location_index());
      }
      continue;
    }

    // We still have both jobs and breaks to go through, so decide on
    // ordering.
    const auto& b = v.breaks[current_break];

    const auto job_action_time = (j.location_index() == current.location_index)
                                   ? j.services[v_type]
                                   : j.setups[v_type] + j.services[v_type];

    auto oc = order_choice(input,
                           *current_job,
                           job_action_time,
                           b,
                           current,
                           next,
                           current_load);

    assert(oc.add_job_first xor oc.add_break_first);
    if (oc.add_break_first) {
      assert(b.is_valid_for_load(current_load));

      if (current.earliest_end < oc.b_tw->start) {
        if (const auto margin = oc.b_tw->start - current.earliest_end;
            margin < current.travel_time_from_prev) {
          current.travel_time_from_prev -= margin;
        } else {
          current.travel_time_from_prev = 0;
        }

        current.earliest_end = oc.b_tw->start;
      }
      break_earliest[current_break] = current.earliest_end;

      current.earliest_end += b.service;

      // Update break max load margin.
      auto current_margin = (b.max_load.has_value())
                              ? b.max_load.value() - current_load
                              : utils::max_amount(input.get_amount_size());
      if (current_break == 0) {
        // New fwd_smallest_breaks_load_margin is solely based on this
        // break max_load.
        fwd_smallest_breaks_load_margin[current_break] = current_margin;
      } else {
        const auto& previous_margin =
          fwd_smallest_breaks_load_margin[current_break - 1];
        for (std::size_t i = 0; i < previous_margin.size(); ++i) {
          fwd_smallest_breaks_load_margin[current_break][i] =
            std::min(previous_margin[i], current_margin[i]);
        }
      }

      ++breaks_before;
      ++current_break;
    }
    if (oc.add_job_first) {
      current.earliest_end =
        std::max(current.earliest_end + current.travel_time_from_prev, oc.j_tw->start);

      route[current_job_rank] = *current_job;
      earliest[current_job_rank] = current.earliest_end;
      breaks_at_rank[current_job_rank] = breaks_before;
      breaks_counts[current_job_rank] = previous_breaks_counts + breaks_before;

      action_time[current_job_rank] = job_action_time;
      current.earliest_end += job_action_time;
      current.location_index = j.location_index();

      ++current_job_rank;
      previous_breaks_counts += breaks_before;
      breaks_before = 0;

      assert(j.delivery <= current_load);
      current_load += (j.pickup - j.delivery);

      ++current_job;
      if (current_job != last_job) {
        // Account for travel time to next current job.
        current.travel_time_from_prev =
          v.duration(j.location_index(), input.jobs[*current_job].location_index());
      }
    }
  }

  assert(current_job_rank == first_rank + add_count);

  // Update all break load margins after modified range.
  const Amount delta_pickup = current_load - previous_final_load;
  // LLM:Update backward load margins for breaks occurring after the modified range.
  // LLM:This is necessary because the pickup amount change affects the load profile
  // LLM:for all subsequent breaks.
  for (std::size_t i = last_break; i < v.breaks.size(); ++i) {
    assert(delta_pickup <= bwd_smallest_breaks_load_margin[i]);

    // Manually decrement margin to avoid overflows that would end up
    // in a negative margin with a plain
    // bwd_smallest_breaks_load_margin[i] -= delta_pickup;
    for (std::size_t a = 0; a < delta_pickup.size(); ++a) {
      if ((-delta_pickup[a]) <= (std::numeric_limits<Capacity>::max() -
                                 bwd_smallest_breaks_load_margin[i][a])) {
        bwd_smallest_breaks_load_margin[i][a] -= delta_pickup[a];
      } else {
        bwd_smallest_breaks_load_margin[i][a] =
          std::numeric_limits<Capacity>::max();
      }
    }
  }

  // Update remaining number of breaks due before next step.
  breaks_at_rank[current_job_rank] = breaks_before;
  assert(previous_breaks_counts + breaks_at_rank[current_job_rank] ==
         breaks_counts[current_job_rank]);

  if (!route.empty()) {
    auto valid_latest_date_rank = current_job_rank;
    auto valid_earliest_date_rank = 0;
    const bool replace_last_jobs = (current_job_rank == route.size());
    bool do_update_last_latest_date = false;

    // LLM:Propagate changes to earliest and latest dates for the rest of the route.
    // LLM:This involves forward propagation of earliest dates and backward propagation of latest dates.
    if (replace_last_jobs) {
      earliest_end = current.earliest_end + next.travel_time_to;

      do_update_last_latest_date = true;
      valid_latest_date_rank = route.size() - 1;
    } else {
      // current_job_rank is the rank of the first non-replaced job.
      const auto& j = input.jobs[route[current_job_rank]];

      const auto new_action_time = (j.location_index() == current.location_index)
                                     ? j.services[v_type]
                                     : j.setups[v_type] + j.services[v_type];
      assert(action_time[current_job_rank] == j.services[v_type] ||
             action_time[current_job_rank] ==
               j.services[v_type] + j.setups[v_type]);

      const bool current_action_time_changed =
        (new_action_time != action_time[current_job_rank]);
      if (current_action_time_changed) {
        // Due to removal, total time spent at first non-replaced
        // task changed, so we need its latest date updated, either
        // directly if at the end of the route, either by going
        // backward from next task (if any).
        if (current_job_rank == route.size() - 1) {
          do_update_last_latest_date = true;
        } else {
          valid_latest_date_rank = current_job_rank + 1;
          // We need to update latest dates for the previous jobs
          // **before** current_job_rank, but bwd_update_latest_from
          // has a stop criterion for propagation that will trigger if
          // latest date happens to not change at current_job_rank.
          latest[current_job_rank] = 0;
        }
      }

      if (current_job_rank == 0) {
        // First jobs in route have been erased and not replaced, so
        // update new first job earliest date and action time.
        current.earliest_end += next.travel_time_to;
        const auto j_tw = std::ranges::find_if(j.tws, [&](const auto& tw) {
          return current.earliest_end <= tw.end;
        });
        assert(j_tw != j.tws.end());

        earliest[0] = std::max(current.earliest_end, j_tw->start);
        assert(earliest[0] <= latest[0] ||
               (current_action_time_changed && latest[current_job_rank] == 0));

        action_time[0] = new_action_time;
      } else {
        valid_earliest_date_rank = current_job_rank - 1;
        if (current_action_time_changed) {
          // We need to update earliest dates for the following jobs
          // **after** current_job_rank, but fwd_update_earliest_from
          // has a stop criterion for propagation that will trigger if
          // earliest date happens to not change at current_job_rank.
          earliest[current_job_rank] = std::numeric_limits<Duration>::max();
        }
      }
    }

    if (!replace_last_jobs) {
      // Update earliest dates forward.
      fwd_update_action_time_from(input, valid_earliest_date_rank);
      fwd_update_earliest_from(input, valid_earliest_date_rank);
    }

    if (do_update_last_latest_date) {
      update_last_latest_date(input);
    }
    // Update latest dates backward.
    bwd_update_latest_from(input, valid_latest_date_rank);
  }

  update_amounts(input);

  // Propagate fwd/bwd_smallest_breaks_load_margin if required.
  if (last_break < v.breaks.size()) {
    fwd_update_breaks_load_margin_from(input, current_job_rank);
  }
  if (last_break > 0) {
    bwd_update_breaks_load_margin_from(input, current_job_rank);
  }
  TRACE_LOG("replace() done\n route:" << this->to_string(&input));

}
 std::ostream& TWRoute::toString(std::ostream& os, [[maybe_unused]] const Input * input) const {
    os << "TWRoute v_rank:" << v_rank << " route_size:" << route.size()
       << " earliest_end:" << earliest_end << std::endl;
    for (std::size_t i = 0; i < route.size(); ++i) {
     
      os << std::setw(2)<< i << " " 
         << " @:"<<t2str(earliest[i] )
         << " job:"<<route[i] ;
      if (input!=nullptr){   
        const auto& j = input->jobs[route[i]];
        os << " loc:" << j.location.index();
      }
      os << std::endl;
      
         
    }
    return os;
  }
// BRUNO: template instantiations follow
template bool
TWRoute::is_valid_addition_for_tw(const Input& input,
                                  const Amount& delivery,
                                  const std::vector<Index>::iterator first_job,
                                  const std::vector<Index>::iterator last_job,
                                  const Index first_rank,
                                  const Index last_rank,
                                  bool check_max_load) const;

template bool TWRoute::is_valid_addition_for_tw(
  const Input& input,
  const Amount& delivery,
  const std::vector<Index>::reverse_iterator first_job,
  const std::vector<Index>::reverse_iterator last_job,
  const Index first_rank,
  const Index last_rank,
  bool check_max_load) const;

template bool TWRoute::is_valid_addition_for_tw(
  const Input& input,
  const Amount& delivery,
  const std::array<Index, 1>::const_iterator first_job,
  const std::array<Index, 1>::const_iterator last_job,
  const Index first_rank,
  const Index last_rank,
  bool check_max_load) const;

template bool TWRoute::is_valid_addition_for_tw(
  const Input& input,
  const Amount& delivery,
  const std::vector<Index>::const_iterator first_job,
  const std::vector<Index>::const_iterator last_job,
  const Index first_rank,
  const Index last_rank,
  bool check_max_load) const;

template void TWRoute::replace(const Input& input,
                               const Amount& delivery,
                               const std::vector<Index>::iterator first_job,
                               const std::vector<Index>::iterator last_job,
                               const Index first_rank,
                               const Index last_rank);
template void
TWRoute::replace(const Input& input,
                 const Amount& delivery,
                 const std::vector<Index>::const_iterator first_job,
                 const std::vector<Index>::const_iterator last_job,
                 const Index first_rank,
                 const Index last_rank);
template void
TWRoute::replace(const Input& input,
                 const Amount& delivery,
                 const std::vector<Index>::reverse_iterator first_job,
                 const std::vector<Index>::reverse_iterator last_job,
                 const Index first_rank,
                 const Index last_rank);

template void
TWRoute::replace(const Input& input,
                 const Amount& delivery,
                 const std::array<Index, 1>::const_iterator first_job,
                 const std::array<Index, 1>::const_iterator last_job,
                 const Index first_rank,
                 const Index last_rank);

} // namespace vroom
