#include "rg/Geometry.h"

#include "rg/BoundingBox.h"

namespace rg {

bool pointInRing(const Ring& ring, double longitude, double latitude) {
  bool inside = false;

  for (std::size_t i = 0, j = ring.size() - 1; i < ring.size(); j = i++) {
    const Coordinate& a = ring[i];
    const Coordinate& b = ring[j];

    const bool intersects =
        ((a.latitude > latitude) != (b.latitude > latitude)) &&
        (longitude < ((b.longitude - a.longitude) * (latitude - a.latitude)) /
                             ((b.latitude - a.latitude) == 0.0 ? 1e-12 : (b.latitude - a.latitude)) +
                         a.longitude);

    if (intersects) {
      inside = !inside;
    }
  }

  return inside;
}

bool pointInPolygon(const Polygon& polygon, double longitude, double latitude) {
  if (!contains(polygon.bbox, longitude, latitude) || polygon.rings.empty()) {
    return false;
  }

  if (!pointInRing(polygon.rings.front(), longitude, latitude)) {
    return false;
  }

  for (std::size_t index = 1; index < polygon.rings.size(); index += 1) {
    if (pointInRing(polygon.rings[index], longitude, latitude)) {
      return false;
    }
  }

  return true;
}

}  // namespace rg
