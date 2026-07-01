#import "RGReverseGeocoder.h"

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "rg/ReverseGeocoder.h"

@implementation RGCountryMetadata

- (instancetype)initWithCountryCode:(NSString *)countryCode
                        countryName:(NSString *)countryName
                           iso3Code:(NSString *)iso3Code
                             status:(NSString *)status
                          continent:(NSString *)continent
                             region:(NSString *)region
                    frenchShortName:(NSString *)frenchShortName {
  self = [super init];
  if (self != nil) {
    _countryCode = [countryCode copy];
    _countryName = [countryName copy];
    _iso3Code = [iso3Code copy];
    _status = [status copy];
    _continent = [continent copy];
    _region = [region copy];
    _frenchShortName = [frenchShortName copy];
  }
  return self;
}

@end

@implementation RGReverseGeocodeResult

- (instancetype)initWithCountry:(RGCountryMetadata *)country
                      stateCode:(NSString *)stateCode
                      stateName:(NSString *)stateName {
  self = [super init];
  if (self != nil) {
    _country = country;
    _stateCode = [stateCode copy];
    _stateName = [stateName copy];
  }
  return self;
}

@end

static NSString *RGStringFromStd(const std::string& value) {
  return [NSString stringWithUTF8String:value.c_str()] ?: @"";
}

static RGCountryMetadata *RGCountryMetadataFromCpp(const rg::CountryMetadata& metadata) {
  return [[RGCountryMetadata alloc] initWithCountryCode:RGStringFromStd(metadata.countryCode)
                                            countryName:RGStringFromStd(metadata.countryName)
                                               iso3Code:RGStringFromStd(metadata.iso3Code)
                                                 status:RGStringFromStd(metadata.status)
                                              continent:RGStringFromStd(metadata.continent)
                                                 region:RGStringFromStd(metadata.region)
                                        frenchShortName:RGStringFromStd(metadata.frenchShortName)];
}

@implementation RGReverseGeocoder {
  std::unique_ptr<rg::ReverseGeocoder> _geocoder;
}

- (instancetype)init {
  self = [super init];
  if (self != nil) {
    _geocoder = std::make_unique<rg::ReverseGeocoder>();
  }
  return self;
}

- (nullable NSString *)reverseGeocodeLatitude:(double)latitude longitude:(double)longitude {
  if (_geocoder == nullptr) {
    return nil;
  }

  const std::optional<std::string> result = _geocoder->reverseGeocode(latitude, longitude);
  if (!result.has_value()) {
    return nil;
  }

  return [NSString stringWithUTF8String:result->c_str()];
}

- (nullable NSString *)reverseGeocodeStateLatitude:(double)latitude longitude:(double)longitude {
  if (_geocoder == nullptr) {
    return nil;
  }

  const std::optional<std::string> result = _geocoder->reverseGeocodeState(latitude, longitude);
  if (!result.has_value()) {
    return nil;
  }

  return [NSString stringWithUTF8String:result->c_str()];
}

- (nullable RGReverseGeocodeResult *)reverseGeocodeResultLatitude:(double)latitude longitude:(double)longitude {
  if (_geocoder == nullptr) {
    return nil;
  }

  const std::optional<rg::ReverseGeocodeResult> result =
      _geocoder->reverseGeocodeResult(latitude, longitude);
  if (!result.has_value()) {
    return nil;
  }

  return [[RGReverseGeocodeResult alloc] initWithCountry:RGCountryMetadataFromCpp(result->country)
                                               stateCode:RGStringFromStd(result->state.stateCode)
                                               stateName:RGStringFromStd(result->state.stateName)];
}

- (nullable RGCountryMetadata *)countryMetadataForCode:(NSString *)countryCode {
  if (_geocoder == nullptr) {
    return nil;
  }

  const std::optional<rg::CountryMetadata> result =
      _geocoder->countryMetadata(std::string(countryCode.UTF8String ?: ""));
  if (!result.has_value()) {
    return nil;
  }

  return RGCountryMetadataFromCpp(result.value());
}

- (NSArray<RGCountryMetadata *> *)allCountryMetadata {
  if (_geocoder == nullptr) {
    return @[];
  }

  const std::vector<rg::CountryMetadata> allMetadata = _geocoder->allCountryMetadata();
  NSMutableArray<RGCountryMetadata *> *items = [NSMutableArray arrayWithCapacity:allMetadata.size()];

  for (const rg::CountryMetadata& metadata : allMetadata) {
    [items addObject:RGCountryMetadataFromCpp(metadata)];
  }

  return items;
}

- (NSUInteger)countryCount {
  if (_geocoder == nullptr) {
    return 0;
  }

  return static_cast<NSUInteger>(_geocoder->countryCount());
}

@end
