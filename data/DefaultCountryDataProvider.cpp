#include "rg/DefaultCountryDataProvider.h"

#include "generated/CountryRegistry.h"

namespace rg {

std::vector<CountryBoundary> DefaultCountryDataProvider::loadCountries() const {
  return loadGeneratedCountries();
}

}  // namespace rg
