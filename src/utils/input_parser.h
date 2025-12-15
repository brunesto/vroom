#ifndef INPUT_PARSER_H
#define INPUT_PARSER_H

/*

This file is part of VROOM.

Copyright (c) 2015-2025, Julien Coupey.
All rights reserved (see LICENSE).

*/

#include "structures/vroom/input/input.h"
#include "structures/vroom/input/vehicle_step.h"

namespace vroom::io {

/**
 * LLM:
 * @brief Parse a JSON string to populate the input object.
 * @param input The Input object to populate.
 * @param input_str The JSON string to parse.
 * @param geometry Whether to include geometry information.
 * @throws InputException If the JSON is invalid or contains invalid data.
 */
void parse(Input& input, const std::string& input_str, bool geometry);

} // namespace vroom::io

#endif
