#include "rg/ReverseGeocoder.h"

#include "rg/BoundingBox.h"
#include "rg/DefaultCountryDataProvider.h"
#include "rg/DefaultUSStateDataProvider.h"
#include "rg/Geometry.h"

namespace rg {

CountryMetadata ReverseGeocoder::makeCountryMetadata(const CountryBoundary& country) {
  return CountryMetadata{
      country.countryCode,
      country.countryName,
      country.iso3Code,
      country.status,
      country.continent,
      country.region,
      country.frenchShortName,
  };
}

StateMetadata ReverseGeocoder::makeStateMetadata(const CountryBoundary& state) {
  StateMetadata metadata;
  metadata.stateName = state.countryName;

  constexpr char kUSStatePrefix[] = "US-";
  constexpr std::size_t kUSStatePrefixLength = sizeof(kUSStatePrefix) - 1;
  if (state.countryCode.rfind(kUSStatePrefix, 0) == 0 && state.countryCode.size() > kUSStatePrefixLength) {
    metadata.stateCode = state.countryCode.substr(kUSStatePrefixLength);
  }

  return metadata;
}

ReverseGeocoder::ReverseGeocoder()
    : ReverseGeocoder(std::make_shared<DefaultCountryDataProvider>(),
                      std::make_shared<DefaultUSStateDataProvider>()) {}

ReverseGeocoder::ReverseGeocoder(std::shared_ptr<const CountryDataProvider> provider) {
  if (provider != nullptr) {
    countryStore_.replaceCountries(provider->loadCountries());
  }
}

ReverseGeocoder::ReverseGeocoder(std::shared_ptr<const CountryDataProvider> countryProvider,
                                 std::shared_ptr<const CountryDataProvider> usStateProvider) {
  if (countryProvider != nullptr) {
    countryStore_.replaceCountries(countryProvider->loadCountries());
  }
  if (usStateProvider != nullptr) {
    usStateStore_.replaceCountries(usStateProvider->loadCountries());
  }
}

std::optional<std::string> ReverseGeocoder::reverseGeocode(double latitude, double longitude) const {
  const std::optional<ReverseGeocodeResult> result = reverseGeocodeResult(latitude, longitude);
  if (!result.has_value()) {
    return std::nullopt;
  }

  return result->country.countryCode;
}

std::optional<std::string> ReverseGeocoder::reverseGeocodeState(double latitude,
                                                                double longitude) const {
  const std::optional<ReverseGeocodeResult> result = reverseGeocodeResult(latitude, longitude);
  if (!result.has_value() || result->state.stateCode.empty()) {
    return std::nullopt;
  }

  return result->state.stateCode;
}

std::optional<ReverseGeocodeResult> ReverseGeocoder::reverseGeocodeResult(double latitude,
                                                                          double longitude) const {
  const std::vector<std::size_t> candidates =
      countryStore_.candidateCountryIndexes(longitude, latitude);
  const std::vector<CountryBoundary>& countries = countryStore_.countries();

  for (std::size_t candidateIndex : candidates) {
    const CountryBoundary& country = countries[candidateIndex];

    if (!contains(country.bbox, longitude, latitude)) {
      continue;
    }

    for (const Polygon& polygon : country.polygons) {
      if (pointInPolygon(polygon, longitude, latitude)) {
        StateMetadata state;
        if (country.countryCode == "US") {
          state = reverseGeocodeUSState(latitude, longitude).value_or(StateMetadata{});
        }
        return ReverseGeocodeResult{makeCountryMetadata(country), state};
      }
    }
  }

  return std::nullopt;
}

std::optional<CountryMetadata> ReverseGeocoder::countryMetadata(const std::string& countryCode) const {
  for (const CountryBoundary& country : countryStore_.countries()) {
    if (country.countryCode == countryCode) {
      return makeCountryMetadata(country);
    }
  }

  return std::nullopt;
}

std::vector<CountryMetadata> ReverseGeocoder::allCountryMetadata() const {
  std::vector<CountryMetadata> metadata;
  metadata.reserve(countryStore_.countries().size());

  for (const CountryBoundary& country : countryStore_.countries()) {
    metadata.push_back(makeCountryMetadata(country));
  }

  return metadata;
}

std::size_t ReverseGeocoder::countryCount() const {
  return countryStore_.countryCount();
}

std::optional<StateMetadata> ReverseGeocoder::reverseGeocodeUSState(double latitude,
                                                                    double longitude) const {
  const std::vector<std::size_t> candidates =
      usStateStore_.candidateCountryIndexes(longitude, latitude);
  const std::vector<CountryBoundary>& states = usStateStore_.countries();

  for (std::size_t candidateIndex : candidates) {
    const CountryBoundary& state = states[candidateIndex];

    if (!contains(state.bbox, longitude, latitude)) {
      continue;
    }

    for (const Polygon& polygon : state.polygons) {
      if (pointInPolygon(polygon, longitude, latitude)) {
        return makeStateMetadata(state);
      }
    }
  }

  return std::nullopt;
}

}  // namespace rg
