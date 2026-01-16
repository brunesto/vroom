#ifndef RETURN_TO_DEPOT_WITH_UNDELIVERED_H
#define RETURN_TO_DEPOT_WITH_UNDELIVERED_H

#include <cstddef>
#include <cstdint>
#include <iterator>
#include "structures/typedefs.h"
#include "structures/vroom/input/input.h"

namespace vroom {

    // default type parameter is int*, so that the default dummies {} are of a given type
  template <std::forward_iterator Iter>
    bool is_return_to_depot_with_undelivered_jobs(const Input& input,const std::vector<Index>* jobs,                                                  
                                                  const Index first_rank=0,
                                                  const Index last_rank=0,  
                                                  const Iter &first_job={},
                                                  const Iter &last_job={},
                                                  const bool bark_when_broken=false     
                                                  ) ;
                                       
 bool is_return_to_depot_with_undelivered_jobs_no_insertion(const Input& input,const std::vector<Index>* jobs,                                                  
                                                  const Index first_rank=0,
                                                  const Index last_rank=0 
                                                  );
}
#endif