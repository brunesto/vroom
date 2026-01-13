#include "extra_opts.h"
#include <string>
#include "utils/log.h"

namespace vroom {
 bool extra_options_no_return_undelivered_to_depot=false;
 void parse_extra_options(std::string extra_options){

 INFO_LOG(" parse_extra_options() extra_options:"<< extra_options);
  extra_options_no_return_undelivered_to_depot = false;
  if (extra_options.find("no-return-undelivered-to-depot") !=std::string::npos) {
      extra_options_no_return_undelivered_to_depot = true;
  }
  INFO_LOG(" extra_options_no_return_undelivered_to_depot:"<< extra_options_no_return_undelivered_to_depot);
 }  
     
 
};