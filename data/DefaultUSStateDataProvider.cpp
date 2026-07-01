#include "rg/DefaultUSStateDataProvider.h"

#include "us_states/generated/USStateRegistry.h"

namespace rg {

std::vector<CountryBoundary> DefaultUSStateDataProvider::loadCountries() const {
  return loadGeneratedUSStates();
}

}  // namespace rg
