#pragma once

#include "rg/Types.h"

namespace rg {

bool contains(const BoundingBox& bbox, double longitude, double latitude);
BoundingBox computeBoundingBox(const Ring& ring);
BoundingBox computeBoundingBox(const Polygon& polygon);
BoundingBox computeBoundingBox(const CountryBoundary& country);

}  // namespace rg
