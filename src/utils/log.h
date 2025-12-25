#ifndef LOG_H
#define LOG_H
#include <iostream>

#define DEBUG_LOG(x) std::cerr << "[DEBUG] " << __FILE__ << ":" << __LINE__ << " -> " << x << std::endl
#define INFO_LOG(x) std::cerr << " [INFO] " << __FILE__ << ":" << __LINE__ << " -> " << x << std::endl
#endif