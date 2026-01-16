/*

This file is part of VROOM.

Copyright (c) 2015-2025, Julien Coupey.
All rights reserved (see LICENSE).

*/

#include "problems/vrptw/operators/intra_relocate.h"

namespace vroom::vrptw {

IntraRelocate::IntraRelocate(const Input& input,
                             const utils::SolutionState& sol_state,
                             TWRoute& tw_s_route,
                             Index s_vehicle,
                             Index s_rank,
                             Index t_rank)
  : cvrp::IntraRelocate(input,
                        sol_state,
                        static_cast<RawRoute&>(tw_s_route),
                        s_vehicle,
                        s_rank,
                        t_rank),
    _tw_s_route(tw_s_route) {
}

bool IntraRelocate::is_valid() {
  return cvrp::IntraRelocate::is_valid() &&
         _tw_s_route.is_valid_addition_for_tw(_input,
                                              _delivery,
                                              _moved_jobs.begin(),
                                              _moved_jobs.end(),
                                              _first_rank,
                                              _last_rank);
}

void IntraRelocate::applyJobsConst(std::vector<Index>& s_route, std::vector<Index>& t_route) const {
//TODO: move here the code from apply() function that only modifies the job arrays s_route and t_route.
}
 
void IntraRelocate::apply() {
//  applyJobs(s_route,t_route);

  _tw_s_route.replace(_input,
                      _delivery,
                      _moved_jobs.begin(),
                      _moved_jobs.end(),
                      _first_rank,
                      _last_rank);
}

std::vector<Index> IntraRelocate::addition_candidates() const {
  return {s_vehicle};
}

} // namespace vroom::vrptw
