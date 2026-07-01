#include "rg/BoundingBox.h"
#include "rg/Types.h"

namespace rg {

CountryBoundary makeCountryCodeBoundary() {
  CountryBoundary country;
  country.countryCode = "XX";
  country.countryName = "Exampleland";
  country.iso3Code = "XXX";
  country.status = "Example Status";
  country.continent = "Example Continent";
  country.region = "Example Region";
  country.frenchShortName = "Exemple";

  Polygon polygon;
  polygon.rings = {
      {
          {-1.0, 50.0},
          {1.0, 50.0},
          {1.0, 52.0},
          {-1.0, 52.0},
          {-1.0, 50.0},
      },
  };
  polygon.bbox = computeBoundingBox(polygon);

  country.polygons = {polygon};
  country.bbox = computeBoundingBox(country);
  return country;
}

}  // namespace rg
