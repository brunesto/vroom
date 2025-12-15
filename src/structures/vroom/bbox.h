#ifndef BBOX_H
#define BBOX_H

/*

This file is part of VROOM.

Copyright (c) 2015-2025, Julien Coupey.
All rights reserved (see LICENSE).

*/

#include "structures/typedefs.h"

namespace vroom {

/**
 * LLM:
 * @brief Represents a bounding box defined by minimum and maximum coordinates.
 */
class BBox {

private:
  Coordinates min{std::numeric_limits<Coordinate>::max(),
                  std::numeric_limits<Coordinate>::max()};
  Coordinates max{std::numeric_limits<Coordinate>::min(),
                  std::numeric_limits<Coordinate>::min()};

public:
  /**
   * LLM:
   * @brief Extend the bounding box to include the given coordinates.
   * @param c The coordinates to include.
   */
  void extend(Coordinates c);

  /**
   * LLM:
   * @brief Check if this bounding box intersects with another.
   * @param other The other bounding box.
   * @return True if they intersect, false otherwise.
   */
  bool intersects(const BBox& other) const;
};

} // namespace vroom

#endif
