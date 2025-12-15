#ifndef LOCATION_H
#define LOCATION_H

/*

This file is part of VROOM.

Copyright (c) 2015-2025, Julien Coupey.
All rights reserved (see LICENSE).

*/

#include <cassert>

#include "structures/typedefs.h"

namespace vroom {

/**
 * LLM:
 * @brief Represents a physical location, defined by an index and/or coordinates.
 */
class Location {
private:
  // Index of this location in the matrix.
  Index _index;
  // Coordinates (not mandatory).
  OptionalCoordinates _coords;
  bool _user_index;

public:
  /**
   * LLM:
   * @brief Constructs a location with a specific index.
   *
   * @param index The index of the location.
   */
  explicit Location(Index index);

  /**
   * LLM:
   * @brief Constructs a location with a specific index and coordinates.
   *
   * @param index The index of the location.
   * @param coords The coordinates of the location.
   */
  Location(Index index, const Coordinates& coords);

  /**
   * LLM:
   * @brief Constructs a location with specific coordinates.
   *
   * @param coords The coordinates of the location.
   */
  explicit Location(const Coordinates& coords);

  /**
   * LLM:
   * @brief Sets the index of the location.
   *
   * @param index The new index.
   */
  void set_index(Index index);

  /**
   * LLM:
   * @brief Checks if the location has coordinates.
   *
   * @return True if coordinates are set, false otherwise.
   */
  bool has_coordinates() const;

  Index index() const {
    return _index;
  }

  /**
   * LLM:
   * @brief Gets the coordinates of the location.
   *
   * @return The coordinates.
   */
  Coordinates coordinates() const;

  /**
   * LLM:
   * @brief Gets the longitude of the location.
   *
   * @return The longitude.
   */
  Coordinate lon() const;

  /**
   * LLM:
   * @brief Gets the latitude of the location.
   *
   * @return The latitude.
   */
  Coordinate lat() const;

  /**
   * LLM:
   * @brief Checks if the index was provided by the user.
   *
   * @return True if the index is user-provided, false otherwise.
   */
  bool user_index() const;

  /**
   * LLM:
   * @brief Checks if two locations are identical.
   *
   * Locations are considered identical if they have the same user-provided index
   * or if they both have coordinates and those are equal.
   *
   * @param other The other location to compare with.
   * @return True if the locations are identical, false otherwise.
   */
  // Locations are considered identical if they have the same
  // user-provided index or if they both have coordinates and those
  // are equal. The last part is required for situations with no
  // explicit index provided in input.
  bool operator==(const Location& other) const;
};

} // namespace vroom

namespace std {
template <> struct hash<vroom::Location> {
  std::size_t operator()(const vroom::Location& l) const noexcept {
    if (l.user_index()) {
      return hash<vroom::Index>()(l.index());
    }

    assert(l.has_coordinates());
    return ((hash<vroom::Coordinate>()(l.lon()) ^
             (hash<vroom::Coordinate>()(l.lat()) << 1)) >>
            1);
  }
};
} // namespace std

#endif
