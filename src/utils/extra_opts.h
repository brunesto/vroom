#ifndef EXTRA_OPTS_H
#define EXTRA_OPTS_H

#include <string>
namespace vroom {
    extern bool extra_options_no_return_undelivered_to_depot ;
    void parse_extra_options(std::string extra_options);
};

#endif
