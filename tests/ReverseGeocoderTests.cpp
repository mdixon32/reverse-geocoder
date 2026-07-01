#include <cassert>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "rg/BoundingBox.h"
#include "rg/CountryDataProvider.h"
#include "rg/EncodedCountryBoundary.h"
#include "rg/ReverseGeocoder.h"

namespace {

void appendUnsignedVarint(std::vector<std::uint8_t>& bytes, std::uint32_t value) {
  while (true) {
    std::uint8_t nextByte = static_cast<std::uint8_t>(value & 0x7fU);
    value >>= 7;
    if (value != 0) {
      bytes.push_back(static_cast<std::uint8_t>(nextByte | 0x80U));
    } else {
      bytes.push_back(nextByte);
      return;
    }
  }
}

void appendSignedVarint(std::vector<std::uint8_t>& bytes, std::int32_t value) {
  const std::uint32_t zigzag = (static_cast<std::uint32_t>(value) << 1U) ^
      static_cast<std::uint32_t>(value >> 31);
  appendUnsignedVarint(bytes, zigzag);
}

void appendCoordinate(std::vector<std::uint8_t>& bytes,
                      std::int32_t longitudeFixed,
                      std::int32_t latitudeFixed,
                      std::int32_t& previousLongitudeFixed,
                      std::int32_t& previousLatitudeFixed,
                      bool firstPoint) {
  if (firstPoint) {
    appendSignedVarint(bytes, longitudeFixed);
    appendSignedVarint(bytes, latitudeFixed);
  } else {
    appendSignedVarint(bytes, longitudeFixed - previousLongitudeFixed);
    appendSignedVarint(bytes, latitudeFixed - previousLatitudeFixed);
  }

  previousLongitudeFixed = longitudeFixed;
  previousLatitudeFixed = latitudeFixed;
}

void testEncodedCountryBoundaryDecoding() {
  std::vector<std::uint8_t> encoded;
  appendUnsignedVarint(encoded, 1);  // one ring
  appendUnsignedVarint(encoded, 4);  // four unique points

  std::int32_t previousLongitudeFixed = 0;
  std::int32_t previousLatitudeFixed = 0;
  appendCoordinate(encoded, -200000, 5000000, previousLongitudeFixed, previousLatitudeFixed, true);
  appendCoordinate(encoded, 0, 5000000, previousLongitudeFixed, previousLatitudeFixed, false);
  appendCoordinate(encoded, 0, 5200000, previousLongitudeFixed, previousLatitudeFixed, false);
  appendCoordinate(encoded, -200000, 5200000, previousLongitudeFixed, previousLatitudeFixed, false);

  const rg::CountryBoundary decoded = rg::decodeCountryBoundary({
      "AA",
      "Alpha",
      "AAA",
      "Test State",
      "Europe",
      "Western Europe",
      "Alpha",
      encoded.data(),
      encoded.size(),
      1,
  });

  assert(decoded.polygons.size() == 1);
  assert(decoded.polygons.front().rings.size() == 1);
  assert(decoded.polygons.front().rings.front().size() == 5);
  assert(decoded.polygons.front().rings.front().front().longitude == -2.0);
  assert(decoded.polygons.front().rings.front().back().longitude == -2.0);
  assert(decoded.bbox.minLongitude == -2.0);
  assert(decoded.bbox.maxLatitude == 52.0);
}

class SampleProvider final : public rg::CountryDataProvider {
 public:
  std::vector<rg::CountryBoundary> loadCountries() const override {
    rg::Polygon alphaPolygon;
    alphaPolygon.rings = {{
        {-2.0, 50.0},
        {0.0, 50.0},
        {0.0, 52.0},
        {-2.0, 52.0},
        {-2.0, 50.0},
    }};
    alphaPolygon.bbox = rg::computeBoundingBox(alphaPolygon);

    rg::CountryBoundary alpha;
    alpha.countryCode = "AA";
    alpha.polygons = {alphaPolygon};
    alpha.countryName = "Alpha";
    alpha.iso3Code = "AAA";
    alpha.status = "Test State";
    alpha.continent = "Europe";
    alpha.region = "Western Europe";
    alpha.frenchShortName = "Alpha";
    alpha.bbox = rg::computeBoundingBox(alpha);

    rg::Polygon betaPolygon;
    betaPolygon.rings = {{
        {1.0, 48.0},
        {4.0, 48.0},
        {4.0, 50.0},
        {1.0, 50.0},
        {1.0, 48.0},
    }};
    betaPolygon.bbox = rg::computeBoundingBox(betaPolygon);

    rg::CountryBoundary beta;
    beta.countryCode = "BB";
    beta.polygons = {betaPolygon};
    beta.countryName = "Beta";
    beta.iso3Code = "BBB";
    beta.status = "Test State";
    beta.continent = "Europe";
    beta.region = "Eastern Europe";
    beta.frenchShortName = "Beta";
    beta.bbox = rg::computeBoundingBox(beta);

    rg::Polygon usaPolygon;
    usaPolygon.rings = {{
        {-125.0, 24.0},
        {-66.0, 24.0},
        {-66.0, 50.0},
        {-125.0, 50.0},
        {-125.0, 24.0},
    }};
    usaPolygon.bbox = rg::computeBoundingBox(usaPolygon);

    rg::CountryBoundary usa;
    usa.countryCode = "US";
    usa.polygons = {usaPolygon};
    usa.countryName = "United States of America";
    usa.iso3Code = "USA";
    usa.status = "Member State";
    usa.continent = "Americas";
    usa.region = "Northern America";
    usa.frenchShortName = "United States of America";
    usa.bbox = rg::computeBoundingBox(usa);

    return {alpha, beta, usa};
  }
};

class SampleUSStateProvider final : public rg::CountryDataProvider {
 public:
  std::vector<rg::CountryBoundary> loadCountries() const override {
    rg::Polygon californiaPolygon;
    californiaPolygon.rings = {{
        {-125.0, 32.0},
        {-114.0, 32.0},
        {-114.0, 42.0},
        {-125.0, 42.0},
        {-125.0, 32.0},
    }};
    californiaPolygon.bbox = rg::computeBoundingBox(californiaPolygon);

    rg::CountryBoundary california;
    california.countryCode = "US-CA";
    california.polygons = {californiaPolygon};
    california.countryName = "California";
    california.bbox = rg::computeBoundingBox(california);

    rg::Polygon newYorkPolygon;
    newYorkPolygon.rings = {{
        {-80.0, 40.0},
        {-71.0, 40.0},
        {-71.0, 45.5},
        {-80.0, 45.5},
        {-80.0, 40.0},
    }};
    newYorkPolygon.bbox = rg::computeBoundingBox(newYorkPolygon);

    rg::CountryBoundary newYork;
    newYork.countryCode = "US-NY";
    newYork.polygons = {newYorkPolygon};
    newYork.countryName = "New York";
    newYork.bbox = rg::computeBoundingBox(newYork);

    return {california, newYork};
  }
};

}  // namespace

int main() {
  testEncodedCountryBoundaryDecoding();

  auto provider = std::make_shared<SampleProvider>();
  auto stateProvider = std::make_shared<SampleUSStateProvider>();
  rg::ReverseGeocoder geocoder(provider, stateProvider);

  assert(geocoder.countryCount() == 3);
  assert(geocoder.reverseGeocode(51.0, -1.0).value_or("") == "AA");
  assert(geocoder.reverseGeocode(49.0, 2.0).value_or("") == "BB");
  assert(geocoder.reverseGeocode(36.0, -119.0).value_or("") == "US");
  assert(!geocoder.reverseGeocode(10.0, 10.0).has_value());
  assert(geocoder.reverseGeocodeState(36.0, -119.0).value_or("") == "CA");
  assert(!geocoder.reverseGeocodeState(51.0, -1.0).has_value());

  const auto alphaResult = geocoder.reverseGeocodeResult(51.0, -1.0);
  assert(alphaResult.has_value());
  assert(alphaResult->country.countryCode == "AA");
  assert(alphaResult->country.countryName == "Alpha");
  assert(alphaResult->country.iso3Code == "AAA");
  assert(alphaResult->state.stateCode.empty());
  assert(alphaResult->state.stateName.empty());

  const auto californiaResult = geocoder.reverseGeocodeResult(36.0, -119.0);
  assert(californiaResult.has_value());
  assert(californiaResult->country.countryCode == "US");
  assert(californiaResult->state.stateCode == "CA");
  assert(californiaResult->state.stateName == "California");

  const auto betaMetadata = geocoder.countryMetadata("BB");
  assert(betaMetadata.has_value());
  assert(betaMetadata->countryName == "Beta");
  assert(betaMetadata->region == "Eastern Europe");

  const auto allMetadata = geocoder.allCountryMetadata();
  assert(allMetadata.size() == 3);

  return 0;
}
