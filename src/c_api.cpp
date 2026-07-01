#include "rg/c_api.h"

#include <memory>
#include <string>

#include "rg/ReverseGeocoder.h"

struct rg_handle {
  std::unique_ptr<rg::ReverseGeocoder> geocoder;
  std::string lastCountryResult;
  std::string lastStateResult;
};

rg_handle* rg_create_default(void) {
  rg_handle* handle = new rg_handle();
  handle->geocoder = std::make_unique<rg::ReverseGeocoder>();
  return handle;
}

void rg_destroy(rg_handle* handle) {
  delete handle;
}

const char* rg_reverse_geocode(rg_handle* handle, double latitude, double longitude) {
  if (handle == nullptr || handle->geocoder == nullptr) {
    return nullptr;
  }

  const std::optional<std::string> result = handle->geocoder->reverseGeocode(latitude, longitude);
  if (!result.has_value()) {
    handle->lastCountryResult.clear();
    return nullptr;
  }

  handle->lastCountryResult = result.value();
  return handle->lastCountryResult.c_str();
}

const char* rg_reverse_geocode_state(rg_handle* handle, double latitude, double longitude) {
  if (handle == nullptr || handle->geocoder == nullptr) {
    return nullptr;
  }

  const std::optional<std::string> result =
      handle->geocoder->reverseGeocodeState(latitude, longitude);
  if (!result.has_value()) {
    handle->lastStateResult.clear();
    return nullptr;
  }

  handle->lastStateResult = result.value();
  return handle->lastStateResult.c_str();
}

size_t rg_country_count(rg_handle* handle) {
  if (handle == nullptr || handle->geocoder == nullptr) {
    return 0;
  }

  return handle->geocoder->countryCount();
}
