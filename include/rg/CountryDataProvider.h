#pragma once

#include <vector>

#include "rg/Types.h"

namespace rg {

class CountryDataProvider {
 public:
  virtual ~CountryDataProvider() = default;
  virtual std::vector<CountryBoundary> loadCountries() const = 0;
};

}  // namespace rg
