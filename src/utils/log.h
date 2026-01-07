#ifndef LOG_H
#define LOG_H
#include <cstring>
#include <iostream>
#include <climits>

inline const char* get_basename(const char* path) {
  const char* file = strrchr(path, '/');
  return file ? file + 1 : path;
}


/**
* simply dump timestamp t in format hh:mm:ss.mmm
 */
inline std::string t2str(int64_t t) {

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

//#define TRACE_LOG(x) ;
//#define DEBUG_LOG(x) ;
#define TTRACE_LOG(x) std::cerr << "⏰ [TTRACE] " << get_basename(__FILE__) << ":" << __LINE__ << " -> " << x << std::endl
#define TRACE_LOG(x) std::cerr << "👾 [TRACE] " << get_basename(__FILE__) << ":" << __LINE__ << " -> " << x << std::endl
#define DEBUG_LOG(x) std::cerr << "🪲 [DEBUG] " << get_basename(__FILE__) << ":" << __LINE__ << " -> " << x << std::endl
#define INFO_LOG(x) std::cerr << " ℹ️ [INFO] " << get_basename(__FILE__) << ":" << __LINE__ << " -> " << x << std::endl
#endif