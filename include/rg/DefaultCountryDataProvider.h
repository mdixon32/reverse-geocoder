#pragma once

#include "rg/CountryDataProvider.h"

namespace rg {

class DefaultCountryDataProvider final : public CountryDataProvider {
 public:
  std::vector<CountryBoundary> loadCountries() const override;
};

}  // namespace rg
