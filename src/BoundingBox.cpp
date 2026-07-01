#include "rg/BoundingBox.h"

#include <algorithm>
#include <stdexcept>

namespace rg {

bool contains(const BoundingBox& bbox, double longitude, double latitude) {
  return longitude >= bbox.minLongitude && longitude <= bbox.maxLongitude &&
         latitude >= bbox.minLatitude && latitude <= bbox.maxLatitude;
}

BoundingBox computeBoundingBox(const Ring& ring) {
  if (ring.empty()) {
    throw std::invalid_argument("ring must contain at least one point");
  }

  BoundingBox bbox{
      ring.front().longitude,
      ring.front().latitude,
      ring.front().longitude,
      ring.front().latitude,
  };

  for (const Coordinate& point : ring) {
    bbox.minLongitude = std::min(bbox.minLongitude, point.longitude);
    bbox.minLatitude = std::min(bbox.minLatitude, point.latitude);
    bbox.maxLongitude = std::max(bbox.maxLongitude, point.longitude);
    bbox.maxLatitude = std::max(bbox.maxLatitude, point.latitude);
  }

  return bbox;
}

BoundingBox computeBoundingBox(const Polygon& polygon) {
  if (polygon.rings.empty()) {
    throw std::invalid_argument("polygon must contain at least one ring");
  }

  BoundingBox bbox = computeBoundingBox(polygon.rings.front());

  for (std::size_t index = 1; index < polygon.rings.size(); index += 1) {
    BoundingBox ringBbox = computeBoundingBox(polygon.rings[index]);
    bbox.minLongitude = std::min(bbox.minLongitude, ringBbox.minLongitude);
    bbox.minLatitude = std::min(bbox.minLatitude, ringBbox.minLatitude);
    bbox.maxLongitude = std::max(bbox.maxLongitude, ringBbox.maxLongitude);
    bbox.maxLatitude = std::max(bbox.maxLatitude, ringBbox.maxLatitude);
  }

  return bbox;
}

BoundingBox computeBoundingBox(const CountryBoundary& country) {
  if (country.polygons.empty()) {
    throw std::invalid_argument("country must contain at least one polygon");
  }

  BoundingBox bbox = computeBoundingBox(country.polygons.front());

  for (std::size_t index = 1; index < country.polygons.size(); index += 1) {
    BoundingBox polygonBbox = computeBoundingBox(country.polygons[index]);
    bbox.minLongitude = std::min(bbox.minLongitude, polygonBbox.minLongitude);
    bbox.minLatitude = std::min(bbox.minLatitude, polygonBbox.minLatitude);
    bbox.maxLongitude = std::max(bbox.maxLongitude, polygonBbox.maxLongitude);
    bbox.maxLatitude = std::max(bbox.maxLatitude, polygonBbox.maxLatitude);
  }

  return bbox;
}

}  // namespace rg
