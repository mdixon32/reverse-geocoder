#pragma once

#include <memory>
#include <optional>
#include <string>

#include "rg/CountryBoundaryStore.h"
#include "rg/CountryDataProvider.h"

namespace rg {

class ReverseGeocoder {
 public:
  ReverseGeocoder();
  explicit ReverseGeocoder(std::shared_ptr<const CountryDataProvider> provider);
  ReverseGeocoder(std::shared_ptr<const CountryDataProvider> countryProvider,
                  std::shared_ptr<const CountryDataProvider> usStateProvider);

  std::optional<std::string> reverseGeocode(double latitude, double longitude) const;
  std::optional<std::string> reverseGeocodeState(double latitude, double longitude) const;
  std::optional<ReverseGeocodeResult> reverseGeocodeResult(double latitude, double longitude) const;
  std::optional<CountryMetadata> countryMetadata(const std::string& countryCode) const;
  std::vector<CountryMetadata> allCountryMetadata() const;
  std::size_t countryCount() const;

 private:
  static CountryMetadata makeCountryMetadata(const CountryBoundary& country);
  static StateMetadata makeStateMetadata(const CountryBoundary& state);
  std::optional<StateMetadata> reverseGeocodeUSState(double latitude, double longitude) const;

  CountryBoundaryStore countryStore_;
  CountryBoundaryStore usStateStore_;
};

}  // namespace rg
