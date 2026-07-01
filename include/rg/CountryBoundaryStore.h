#pragma once

#include <unordered_map>
#include <vector>

#include "rg/Types.h"

namespace rg {

class CountryBoundaryStore {
 public:
  static constexpr int kDefaultCellSizeDegrees = 2;

  explicit CountryBoundaryStore(int cellSizeDegrees = kDefaultCellSizeDegrees);

  void replaceCountries(std::vector<CountryBoundary> countries);
  const std::vector<CountryBoundary>& countries() const;
  std::vector<std::size_t> candidateCountryIndexes(double longitude, double latitude) const;
  std::size_t countryCount() const;

 private:
  std::string makeCellKey(double longitude, double latitude) const;
  void rebuildIndex();

  int cellSizeDegrees_;
  std::vector<CountryBoundary> countries_;
  std::unordered_map<std::string, std::vector<std::size_t>> cellToCountryIndexes_;
};

}  // namespace rg
