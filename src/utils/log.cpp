#include "utils/log.h"


namespace vroom {


    const bool TTRACE_ENABLED = false;
    const bool TRACE_ENABLED = false;
    const bool DEBUG_ENABLED = false;
    const bool INFO_ENABLED = true;
    const bool WARN_ENABLED = true;    
    const bool ERROR_ENABLED = true;


    const char* get_basename(const char* path) {
    const char* file = strrchr(path, '/');
    return file ? file + 1 : path;
    }

    std::string t2str(int64_t t) {

    const int TIME_PRECISION=100;
    if (t==((int64_t)UINT_MAX)*TIME_PRECISION) {
        return "__MAX_TIME__";            
    }

    int h=t/(3600*TIME_PRECISION);
    t=t%(3600*TIME_PRECISION);

    int m=t/(60*TIME_PRECISION);
    t=t%(60*TIME_PRECISION);

    int s=t/(TIME_PRECISION);
    t=t%(TIME_PRECISION);
    
    char buffer[24];
    snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d.%03d", h,
                m, s,(int)t);
    return std::string(buffer);
    }

}