#ifndef LOG_H
#define LOG_H
#include <iostream>
#define TRACE_LOG(x) std::cerr << "👾 [TRACE] " << __FILE__ << ":" << __LINE__ << " -> " << x << std::endl
#define DEBUG_LOG(x) std::cerr << "🪲 [DEBUG] " << __FILE__ << ":" << __LINE__ << " -> " << x << std::endl
#define INFO_LOG(x) std::cerr << " ℹ️ [INFO] " << __FILE__ << ":" << __LINE__ << " -> " << x << std::endl
#endif