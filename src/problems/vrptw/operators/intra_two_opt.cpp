/*

This file is part of VROOM.

Copyright (c) 2015-2025, Julien Coupey.
All rights reserved (see LICENSE).

*/

#include "problems/vrptw/operators/intra_two_opt.h"

namespace vroom::vrptw {

IntraTwoOpt::IntraTwoOpt(const Input& input,
                         const utils::SolutionState& sol_state,
                         TWRoute& tw_s_route,
                         Index s_vehicle,
                         Index s_rank,
                         Index t_rank)
  : cvrp::IntraTwoOpt(input,
                      sol_state,
                      static_cast<RawRoute&>(tw_s_route),
                      s_vehicle,
                      s_rank,
                      t_rank),
    _tw_s_route(tw_s_route) {
}

bool IntraTwoOpt::is_valid() {
  bool valid = cvrp::IntraTwoOpt::is_valid();

  if (valid) {
    auto rev_t = s_route.rbegin() + (s_route.size() - t_rank - 1);
    auto rev_s_next = s_route.rbegin() + (s_route.size() - s_rank);

    valid = _tw_s_route.is_valid_addition_for_tw(_input,
                                                 delivery,
                                                 rev_t,
                                                 rev_s_next,
                                                 s_rank,
                                                 t_rank + 1);
  }

  return valid;
}

void IntraTwoOpt::applyJobs(std::vector<Index>& s_route, std::vector<Index>& t_route) {
//TODO: move here the code from apply() function that only modifies the job arrays s_route and t_route.
}
 
void IntraTwoOpt::apply() {
//  applyJobs(s_route,t_route);

  std::vector<Index> reversed(s_route.rbegin() + (s_route.size() - t_rank - 1),
                              s_route.rbegin() + (s_route.size() - s_rank));

  _tw_s_route.replace(_input,
                      delivery,
                      reversed.begin(),
                      reversed.end(),
                      s_rank,
                      t_rank + 1);
}

} // namespace vroom::vrptw
