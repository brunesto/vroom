#ifndef EXCEPTION_H
#define EXCEPTION_H

/*

This file is part of VROOM.

Copyright (c) 2015-2025, Julien Coupey.
All rights reserved (see LICENSE).

*/

#include <string>

#include "structures/typedefs.h"

namespace vroom {

/**
 * LLM:
 * @brief Base exception class for VROOM.
 */
class Exception : public std::exception {
public:
  const std::string message;
  unsigned int error_code;

  Exception(std::string message, unsigned int error_code);

  const char* what() const noexcept override {
    return message.c_str();
  };
};

/**
 * LLM:
 * @brief Exception thrown when an internal error occurs.
 */
class InternalException : public Exception {
public:
  explicit InternalException(const std::string& message);
};

/**
 * LLM:
 * @brief Exception thrown when an input error occurs.
 */
class InputException : public Exception {
public:
  explicit InputException(const std::string& message);
};

/**
 * LLM:
 * @brief Exception thrown when a routing error occurs.
 */
class RoutingException : public Exception {
public:
  explicit RoutingException(const std::string& message);
};

} // namespace vroom

#endif
