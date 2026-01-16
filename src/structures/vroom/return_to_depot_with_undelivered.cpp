#include "utils/log.h"
#include "utils/extra_opts.h"

#include "structures/vroom/return_to_depot_with_undelivered.h"

namespace vroom {



 /**
 * this is a helper class to navigate a route with an insertion
 */
template <std::forward_iterator Iter>
class RouteWithInsertion{
   public:
   const Index first_rank;
    const Iter first_job;
    const Iter last_job;
    int size;
    int inserted_size;
    const std::vector<Index> *route;
   
  RouteWithInsertion(
    const std::vector<Index> *route,
    const Index first_rank,
    const Iter &first_job,
    const Iter &last_job)// 3. Use Member Initializer List for const members
    : route(route),
      first_rank(first_rank), 
      first_job(first_job), 
      last_job(last_job),
      size(route->size()+std::distance(first_job, last_job)),
      inserted_size(std::distance(first_job, last_job))
  {      assert(first_job <= last_job);
    }

  bool is_inserted(int i) const{
    if (i<first_rank){
      return false;
    } 
    if (i< first_rank + inserted_size){
      return true;
    }
    return false;
  }  
  int get(int i) const {
    if (i<first_rank){
      return (*route)[i];
    } 
    if (i< first_rank + inserted_size){
      auto offset=i - first_rank;
      return *(first_job + (offset));
    }
    return (*route)[i - inserted_size];
    
  }

  std::string to_string([[maybe_unused]]const Input* input = nullptr) const {
    std::ostringstream os;
   
    os << "RouteWithInsertion size:" << size << std::endl;
    for (int i = 0; i < size; ++i) {
      os << i << " " << std::setw(8) << std::setfill('0') << get(i) ;
      os << (is_inserted(i) ? " (inserted)" : "(existing)");
      os << std::endl;
    }
     return os.str();
  }

};




// // definitely super dirty ... after years in java I miss freedom :P
// #define CHECK_CURRENT_LOCATION(x,reason) bool pjAtDepot=(x)==0; \
//                 if (atDepot!=pjAtDepot){ \
//                   TTRACE_LOG(" depot changed to "<<pjAtDepot<<", because of "<< reason); \
//                   depotChange++;\
//                   if (depotChange>1){\
//                     return true;\
//                   }\
//                   atDepot=pjAtDepot;\
//                 }   

 // NOTE: if first_job and last_job would be not references,
 // the iterators are copied and somehow stop working , i dont know why.
  template <std::forward_iterator Iter>
  bool is_return_to_depot_with_undelivered_jobs(const Input& input,const std::vector<Index>* jobs,
                                                  const Index first_rank,
                                                  const Index last_rank,  
                                                  const Iter& first_job,
                                                  const Iter& last_job,
                                                  const bool bark_when_broken     
                                                  ) {

  if (!extra_options_no_return_undelivered_to_depot){
    return false;
  }

  RouteWithInsertion ri(jobs, first_rank, first_job, last_job);

  // bunch of assertions, non-exhaustive

  // WTF is last rank for?

  // int rank_diff=first_rank-last_rank;
  // if (rank_diff+2!=ri.inserted_size){
  //   ERROR_LOG("rank_diff:"<< rank_diff << " inserted_size:"<< ri.inserted_size);
  // }

  // if (ri.inserted_size==2){
  //   auto first_job_id=*first_job;
  //   auto last_job_id=*(first_job + (1));
    
  //   const auto& fj = input.jobs[first_job_id];
    
  //   const auto& lj = input.jobs[last_job_id];
  //   if (fj.type == JOB_TYPE::PICKUP) {
  //     assert(first_job_id+1==last_job_id);      
  //   }else if (fj.type == JOB_TYPE::DELIVERY) {
  //     assert(first_job_id==last_job_id+1);
  //   }
  // } 
   
  TRACE_LOG(" is_return_to_depot_with_undelivered_jobs()"<< ri.to_string(&input));

  for (int r = 0; r < ri.size; r++) {
    uint16_t rjobId = ri.get(r);
    if (rjobId==50016){
       ri.get(r);
      RouteWithInsertion r2(jobs, first_rank, first_job, last_job);
    }
    const auto& rj = input.jobs[rjobId];

    TTRACE_LOG(" RouteWithInsertion["<<r<<"] ("<<(ri.is_inserted(r)?"inserted":"existing")<<") jobdId:"<< rjobId << " of type "<< static_cast<int>(rj.type)<< " @ location "<<rj.location_index());

    if (rj.type == JOB_TYPE::DELIVERY) {
     
        // track the number of time we enter or leave the depot
      int depotChange=0;

      // atDepot indicates if current location is at depot
      bool atDepot=rj.location_index()==0;

       // s will be the backward scan index
      int s;
      // now identify the matching pickup
      for (s = r - 1; s >= 0; s--) {
        uint16_t sjobId = ri.get(s);
        const auto& sj = input.jobs[sjobId];
        TTRACE_LOG("    bwd check RouteWithInsertion["<<s<<"] ("<<(ri.is_inserted(s)?"inserted":"existing")<<") jobdId:"<< sjobId <<"@ location:"<< sj.location_index()<< " of type "<< static_cast<int>(sj.type) )

        bool sAtDepot=(sj.location_index())==0;
        if (sAtDepot!=atDepot){ 
          TTRACE_LOG(" depot changed to "<<sAtDepot); 
          depotChange++;
          if (depotChange>1){
            return true;
          }
          atDepot=sAtDepot;
        }   
      
        
        if (sj.type == JOB_TYPE::PICKUP) {
          // BRUNO: is this the matching pickup?  i am not sure about the predicate...
          // in my tests it is, what about debug real case
          const auto pd_match = (sjobId+ 1 == rjobId);
          if (pd_match) {                 
            break;
          }
        }
        // if we hit s==0 at this point, it means no matching pickup found, broken state
        if (bark_when_broken)
          assert(s!=0);  
      }
     
    }
  } // end of main loop r


  return false;
}
bool is_return_to_depot_with_undelivered_jobs_no_insertion (const Input& input, const   std::vector<Index>* jobs,                                              
                                                  const Index first_rank,
                                                  const Index last_rank) {
  TRACE_LOG(" is_return_to_depot_with_undelivered_jobs_no_insertion()");
  // after hours of fighting against compiler and linker...
  // we need to instanciate (force) the templates with concrete types that actually depends on tw_route and not raw_route
  // ... what is wrong with using short* ?
  return is_return_to_depot_with_undelivered_jobs< std::vector<Index>::const_iterator >(input,jobs,first_rank, last_rank,{},{});
}


template bool is_return_to_depot_with_undelivered_jobs(
  const Input& input, const   std::vector<Index>* jobs, 
  const Index first_rank,
  const Index last_rank,  
   const std::vector<Index>::iterator &first_job,
   const std::vector<Index>::iterator &last_job,
   const bool bark_when_broken) ;


template bool is_return_to_depot_with_undelivered_jobs(
  const Input& input,  const  std::vector<Index>* jobs, 
  const Index first_rank,
  const Index last_rank,  
  const std::vector<Index>::const_iterator &first_job,
  const std::vector<Index>::const_iterator &last_job,
   const bool bark_when_broken) ;


template bool is_return_to_depot_with_undelivered_jobs(
  const Input& input,const    std::vector<Index>* jobs, 
  const Index first_rank,
  const Index last_rank,  
  const std::vector<Index>::reverse_iterator &first_job,
  const std::vector<Index>::reverse_iterator &last_job,
   const bool bark_when_broken) ;


  template bool is_return_to_depot_with_undelivered_jobs(
  const Input& input, const   std::vector<Index>* jobs, 
  const Index first_rank,
  const Index last_rank,  
  const std::array<Index, 1>::const_iterator &first_job,
  const std::array<Index, 1>::const_iterator &last_job,
   const bool bark_when_broken) ;
       
template bool is_return_to_depot_with_undelivered_jobs(
  const Input& input,  const  std::vector<Index>* jobs, 
  const Index first_rank,
  const Index last_rank,  
  const std::array<Index, 1>::reverse_iterator &first_job,
  const std::array<Index, 1>::reverse_iterator &last_job,
   const bool bark_when_broken) ;

// template bool is_return_to_depot_with_undelivered_jobs(
//   const Input& input,   const std::vector<Index>* jobs, 
//   const Index first_rank,
//   const Index last_rank,  
//   const std::array<Index, 1>::reverse_iterator &first_job,
//   const std::array<Index, 1>::reverse_iterator &last_job,
//    const bool bark_when_broken) ;

}