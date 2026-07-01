#include "rg/EncodedCountryBoundary.h"

namespace rg {
namespace {

const std::uint8_t kPolygonData[] = {
    0x01, 0x05, 0xb2, 0x9d, 0x5a, 0xa6, 0xe4, 0x95, 0x04, 0x8d, 0x01, 0x8c, 0x22, 0xc2, 0x3b, 0xca,
    0x1d, 0x8c, 0x10, 0xeb, 0x10, 0xd9, 0x1d, 0xf7, 0x1f,
};

}  // namespace

CountryBoundary makeCountryBoundaryMC() {
  return decodeCountryBoundary({
      "MC",
      "Monaco",
      "MCO",
      "Member State",
      "Europe",
      "Western Europe",
      "Monaco",
      kPolygonData,
      sizeof(kPolygonData),
      1,
  });
}

}  // namespace rg
