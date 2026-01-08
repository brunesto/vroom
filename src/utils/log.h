#ifndef LOG_H
#define LOG_H
#include <cstring>
#include <iostream>
#include <climits>

namespace vroom {
/**
 * extract basename from a path
 */
const char* get_basename(const char* path);

/**
* simply dump timestamp t in format hh:mm:ss.mmm
 */
std::string t2str(int64_t t);

// use actual variables to avoid recompiling most files when changing log level
extern const bool TRACE_ENABLED ;
extern const bool DEBUG_ENABLED ;
extern const bool TTRACE_ENABLED ;
extern const bool INFO_ENABLED ;

}

// it seems that macros are not part of a namespace, even if they would be defined inside it
#define TTRACE_LOG(x) if (vroom::TTRACE_ENABLED) {std::cerr << "⏰ [TTRACE] " << vroom::get_basename(__FILE__) << ":" << __LINE__ << " -> " << x << std::endl;} else {}
#define TRACE_LOG(x) if (vroom::TRACE_ENABLED){std::cerr << "👾 [TRACE] " << vroom::get_basename(__FILE__) << ":" << __LINE__ << " -> " << x << std::endl;} else {}
#define DEBUG_LOG(x) if (vroom::DEBUG_ENABLED){std::cerr << "🪲 [DEBUG] " << vroom::get_basename(__FILE__) << ":" << __LINE__ << " -> " << x << std::endl;} else {}
#define INFO_LOG(x) if (vroom::INFO_ENABLED){std::cerr << " ℹ️ [INFO] " << vroom::get_basename(__FILE__) << ":" << __LINE__ << " -> " << x << std::endl;} else {}


#endif
