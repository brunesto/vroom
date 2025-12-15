#ifndef OUTPUT_JSON_H
#define OUTPUT_JSON_H

/*

This file is part of VROOM.

Copyright (c) 2015-2025, Julien Coupey.
All rights reserved (see LICENSE).

*/

#include "../include/rapidjson/include/rapidjson/document.h"
#include "structures/vroom/solution/solution.h"
#include "utils/exception.h"

namespace vroom::io {

/**
 * LLM:
 * @brief Convert a Solution object to a RapidJSON document.
 * @param sol The solution to convert.
 * @param report_distances Whether to include distances in the output.
 * @return The RapidJSON document representing the solution.
 */
rapidjson::Document to_json(const Solution& sol, bool report_distances);

/**
 * LLM:
 * @brief Convert an Exception object to a RapidJSON document.
 * @param e The exception to convert.
 * @return The RapidJSON document representing the exception.
 */
rapidjson::Document to_json(const vroom::Exception& e);

/**
 * LLM:
 * @brief Convert a Summary object to a RapidJSON value.
 * @param summary The summary to convert.
 * @param report_distances Whether to include distances in the output.
 * @param allocator The allocator for the RapidJSON document.
 * @return The RapidJSON value representing the summary.
 */
rapidjson::Value to_json(const Summary& summary,
                         bool report_distances,
                         rapidjson::Document::AllocatorType& allocator);

/**
 * LLM:
 * @brief Convert a ComputingTimes object to a RapidJSON value.
 * @param ct The computing times to convert.
 * @param allocator The allocator for the RapidJSON document.
 * @return The RapidJSON value representing the computing times.
 */
rapidjson::Value to_json(const ComputingTimes& ct,
                         rapidjson::Document::AllocatorType& allocator);

/**
 * LLM:
 * @brief Convert a Route object to a RapidJSON value.
 * @param route The route to convert.
 * @param report_distances Whether to include distances in the output.
 * @param allocator The allocator for the RapidJSON document.
 * @return The RapidJSON value representing the route.
 */
rapidjson::Value to_json(const Route& route,
                         bool report_distances,
                         rapidjson::Document::AllocatorType& allocator);

/**
 * LLM:
 * @brief Convert a Step object to a RapidJSON value.
 * @param s The step to convert.
 * @param report_distances Whether to include distances in the output.
 * @param allocator The allocator for the RapidJSON document.
 * @return The RapidJSON value representing the step.
 */
rapidjson::Value to_json(const Step& s,
                         bool report_distances,
                         rapidjson::Document::AllocatorType& allocator);

/**
 * LLM:
 * @brief Convert a Location object to a RapidJSON value.
 * @param loc The location to convert.
 * @param allocator The allocator for the RapidJSON document.
 * @return The RapidJSON value representing the location.
 */
rapidjson::Value to_json(const Location& loc,
                         rapidjson::Document::AllocatorType& allocator);

/**
 * LLM:
 * @brief Write an exception to a JSON file or stdout.
 * @param e The exception to write.
 * @param output_file The path to the output file (empty for stdout).
 */
void write_to_json(const vroom::Exception& e,
                   const std::string& output_file = "");

/**
 * LLM:
 * @brief Write a solution to a JSON file or stdout.
 * @param sol The solution to write.
 * @param output_file The path to the output file (empty for stdout).
 * @param report_distances Whether to include distances in the output.
 */
void write_to_json(const Solution& sol,
                   const std::string& output_file = "",
                   bool report_distances = false);
} // namespace vroom::io

#endif
