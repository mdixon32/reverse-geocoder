#include "rg/CountryBoundaryStore.h"

#include <cmath>
#include <sstream>
#include <stdexcept>

namespace rg {

CountryBoundaryStore::CountryBoundaryStore(int cellSizeDegrees)
    : cellSizeDegrees_(cellSizeDegrees) {
  if (cellSizeDegrees_ <= 0) {
    throw std::invalid_argument("cellSizeDegrees must be positive");
  }
}

void CountryBoundaryStore::replaceCountries(std::vector<CountryBoundary> countries) {
  countries_ = std::move(countries);
  rebuildIndex();
}

const std::vector<CountryBoundary>& CountryBoundaryStore::countries() const {
  return countries_;
}

std::vector<std::size_t> CountryBoundaryStore::candidateCountryIndexes(double longitude,
                                                                       double latitude) const {
  const auto iterator = cellToCountryIndexes_.find(makeCellKey(longitude, latitude));
  if (iterator == cellToCountryIndexes_.end()) {
    return {};
  }

  return iterator->second;
}

std::size_t CountryBoundaryStore::countryCount() const {
  return countries_.size();
}

std::string CountryBoundaryStore::makeCellKey(double longitude, double latitude) const {
  const int lonIndex = static_cast<int>(std::floor((longitude + 180.0) / cellSizeDegrees_));
  const int latIndex = static_cast<int>(std::floor((latitude + 90.0) / cellSizeDegrees_));

  std::ostringstream stream;
  stream << lonIndex << ":" << latIndex;
  return stream.str();
}

void CountryBoundaryStore::rebuildIndex() {
  cellToCountryIndexes_.clear();

  for (std::size_t countryIndex = 0; countryIndex < countries_.size(); countryIndex += 1) {
    const CountryBoundary& country = countries_[countryIndex];
    const int minLonIndex =
        static_cast<int>(std::floor((country.bbox.minLongitude + 180.0) / cellSizeDegrees_));
    const int maxLonIndex =
        static_cast<int>(std::floor((country.bbox.maxLongitude + 180.0) / cellSizeDegrees_));
    const int minLatIndex =
        static_cast<int>(std::floor((country.bbox.minLatitude + 90.0) / cellSizeDegrees_));
    const int maxLatIndex =
        static_cast<int>(std::floor((country.bbox.maxLatitude + 90.0) / cellSizeDegrees_));

    for (int lonIndex = minLonIndex; lonIndex <= maxLonIndex; lonIndex += 1) {
      for (int latIndex = minLatIndex; latIndex <= maxLatIndex; latIndex += 1) {
        std::ostringstream stream;
        stream << lonIndex << ":" << latIndex;
        cellToCountryIndexes_[stream.str()].push_back(countryIndex);
      }
    }
  }
}

}  // namespace rg
