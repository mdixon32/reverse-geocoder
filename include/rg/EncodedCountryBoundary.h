#pragma once

#include <cstddef>
#include <cstdint>

#include "rg/Types.h"

namespace rg {

struct EncodedCountryView {
  const char* countryCode;
  const char* countryName;
  const char* iso3Code;
  const char* status;
  const char* continent;
  const char* region;
  const char* frenchShortName;
  const std::uint8_t* polygonData;
  std::size_t polygonDataSize;
  std::size_t polygonCount;
};

CountryBoundary decodeCountryBoundary(const EncodedCountryView& encoded);

}  // namespace rg
