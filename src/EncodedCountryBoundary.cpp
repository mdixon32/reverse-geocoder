#include "rg/EncodedCountryBoundary.h"

#include "rg/BoundingBox.h"

#include <cstdint>
#include <stdexcept>

namespace rg {

namespace {

constexpr double kCoordinateScale = 100000.0;

class ByteReader {
 public:
  ByteReader(const std::uint8_t* data, std::size_t size) : data_(data), size_(size) {}

  std::uint32_t readUnsignedVarint() {
    std::uint32_t value = 0;
    std::uint32_t shift = 0;

    while (true) {
      if (offset_ >= size_) {
        throw std::runtime_error("unexpected end of encoded polygon data");
      }

      const std::uint8_t byte = data_[offset_++];
      value |= static_cast<std::uint32_t>(byte & 0x7fU) << shift;
      if ((byte & 0x80U) == 0) {
        return value;
      }

      shift += 7;
      if (shift >= 35) {
        throw std::runtime_error("encoded polygon varint overflow");
      }
    }
  }

  std::int32_t readSignedVarint() {
    const std::uint32_t encoded = readUnsignedVarint();
    return static_cast<std::int32_t>((encoded >> 1U) ^ (~(encoded & 1U) + 1U));
  }

  bool empty() const { return offset_ == size_; }

 private:
  const std::uint8_t* data_;
  std::size_t size_;
  std::size_t offset_ = 0;
};

Coordinate decodeCoordinate(std::int32_t longitudeFixed, std::int32_t latitudeFixed) {
  return Coordinate{
      static_cast<double>(longitudeFixed) / kCoordinateScale,
      static_cast<double>(latitudeFixed) / kCoordinateScale,
  };
}

Ring decodeRing(ByteReader& reader) {
  const std::size_t pointCount = reader.readUnsignedVarint();
  if (pointCount < 3) {
    throw std::runtime_error("encoded ring must contain at least three points");
  }

  Ring ring;
  ring.reserve(pointCount + 1);

  std::int32_t longitudeFixed = reader.readSignedVarint();
  std::int32_t latitudeFixed = reader.readSignedVarint();
  ring.push_back(decodeCoordinate(longitudeFixed, latitudeFixed));

  for (std::size_t index = 1; index < pointCount; index += 1) {
    longitudeFixed += reader.readSignedVarint();
    latitudeFixed += reader.readSignedVarint();
    ring.push_back(decodeCoordinate(longitudeFixed, latitudeFixed));
  }

  if (ring.front().longitude != ring.back().longitude ||
      ring.front().latitude != ring.back().latitude) {
    ring.push_back(ring.front());
  }

  return ring;
}

Polygon decodePolygon(ByteReader& reader) {
  const std::size_t ringCount = reader.readUnsignedVarint();
  if (ringCount == 0) {
    throw std::runtime_error("encoded polygon must contain at least one ring");
  }

  Polygon polygon;
  polygon.rings.reserve(ringCount);

  for (std::size_t index = 0; index < ringCount; index += 1) {
    polygon.rings.push_back(decodeRing(reader));
  }

  polygon.bbox = computeBoundingBox(polygon);
  return polygon;
}

}  // namespace

CountryBoundary decodeCountryBoundary(const EncodedCountryView& encoded) {
  CountryBoundary country;
  country.countryCode = encoded.countryCode;
  country.countryName = encoded.countryName;
  country.iso3Code = encoded.iso3Code;
  country.status = encoded.status;
  country.continent = encoded.continent;
  country.region = encoded.region;
  country.frenchShortName = encoded.frenchShortName;
  country.polygons.reserve(encoded.polygonCount);
  ByteReader reader(encoded.polygonData, encoded.polygonDataSize);

  for (std::size_t index = 0; index < encoded.polygonCount; index += 1) {
    country.polygons.push_back(decodePolygon(reader));
  }

  if (!reader.empty()) {
    throw std::runtime_error("encoded polygon data contained trailing bytes");
  }

  country.bbox = computeBoundingBox(country);
  return country;
}

}  // namespace rg
