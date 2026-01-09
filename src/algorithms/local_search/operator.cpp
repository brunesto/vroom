/*

This file is part of VROOM.

Copyright (c) 2015-2025, Julien Coupey.
All rights reserved (see LICENSE).

*/

#include "algorithms/local_search/operator.h"

namespace vroom::ls {

OperatorName Operator::get_name() const {
  return _name;
}

Eval Operator::gain() {
  if (!gain_computed) {
    this->compute_gain();
  }
  return stored_gain;
}

bool Operator::is_valid_for_source_range_bounds() const {
  const auto& s_v = _input.vehicles[s_vehicle];
  return s_v.ok_for_range_bounds(_sol_state.route_evals[s_vehicle] - s_gain);
}

bool Operator::is_valid_for_target_range_bounds() const {
  const auto& t_v = _input.vehicles[t_vehicle];
  return t_v.ok_for_range_bounds(_sol_state.route_evals[t_vehicle] - t_gain);
}

bool Operator::is_valid_for_range_bounds() const {

  

  assert(s_vehicle == t_vehicle);
  assert(gain_computed);

  const auto& s_v = _input.vehicles[s_vehicle];
  return s_v.ok_for_range_bounds(_sol_state.route_evals[s_vehicle] -
                                 stored_gain);
}

std::vector<Index> Operator::required_unassigned() const {
  return std::vector<Index>();
}

bool Operator::invalidated_by(Index) const {
  return false;
}

bool Operator::is_valid2(){
    bool retVal = is_valid();

    if (!retVal)
      return false;
    bool depot_check_target= target.is_no_return_to_depot_with_undelivered_jobs(_input, t_rank);
    if (!depot_check_target)
      return false;
    bool depot_check_src= source.is_no_return_to_depot_with_undelivered_jobs(_input, s_rank);
    return depot_check_src;
   
    //TRACE_LOG("operator "<< _name <<" is_valid: "<< retVal);
  }

} // namespace vroom::ls
