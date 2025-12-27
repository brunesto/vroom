#ifndef LOG_H
#define LOG_H
#include <cstring>
#include <iostream>


inline const char* get_basename(const char* path) {
  const char* file = strrchr(path, '/');
  return file ? file + 1 : path;
}

#define TRACE_LOG(x) std::cerr << "👾 [TRACE] " << get_basename(__FILE__) << ":" << __LINE__ << " -> " << x << std::endl
#define DEBUG_LOG(x) std::cerr << "🪲 [DEBUG] " << get_basename(__FILE__) << ":" << __LINE__ << " -> " << x << std::endl
#define INFO_LOG(x) std::cerr << " ℹ️ [INFO] " << get_basename(__FILE__) << ":" << __LINE__ << " -> " << x << std::endl
#endif