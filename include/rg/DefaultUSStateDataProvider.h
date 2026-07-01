#pragma once

#include "rg/CountryDataProvider.h"

namespace rg {

class DefaultUSStateDataProvider final : public CountryDataProvider {
 public:
  std::vector<CountryBoundary> loadCountries() const override;
};

}  // namespace rg
