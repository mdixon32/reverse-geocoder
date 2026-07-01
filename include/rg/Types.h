#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

namespace rg {

struct Coordinate {
  double longitude = 0.0;
  double latitude = 0.0;
};

struct BoundingBox {
  double minLongitude = 0.0;
  double minLatitude = 0.0;
  double maxLongitude = 0.0;
  double maxLatitude = 0.0;
};

using Ring = std::vector<Coordinate>;

struct Polygon {
  BoundingBox bbox;
  std::vector<Ring> rings;
};

struct CountryMetadata {
  std::string countryCode;
  std::string countryName;
  std::string iso3Code;
  std::string status;
  std::string continent;
  std::string region;
  std::string frenchShortName;
};

struct StateMetadata {
  std::string stateCode;
  std::string stateName;
};

struct CountryBoundary {
  std::string countryCode;
  std::string countryName;
  std::string iso3Code;
  std::string status;
  std::string continent;
  std::string region;
  std::string frenchShortName;
  BoundingBox bbox;
  std::vector<Polygon> polygons;
};

struct ReverseGeocodeResult {
  CountryMetadata country;
  StateMetadata state;
};

}  // namespace rg
