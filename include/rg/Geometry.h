#pragma once

#include "rg/Types.h"

namespace rg {

bool pointInRing(const Ring& ring, double longitude, double latitude);
bool pointInPolygon(const Polygon& polygon, double longitude, double latitude);

}  // namespace rg
