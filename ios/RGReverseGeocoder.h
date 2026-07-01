#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface RGCountryMetadata : NSObject

@property (nonatomic, copy, readonly) NSString *countryCode;
@property (nonatomic, copy, readonly) NSString *countryName;
@property (nonatomic, copy, readonly) NSString *iso3Code;
@property (nonatomic, copy, readonly) NSString *status;
@property (nonatomic, copy, readonly) NSString *continent;
@property (nonatomic, copy, readonly) NSString *region;
@property (nonatomic, copy, readonly) NSString *frenchShortName;

- (instancetype)initWithCountryCode:(NSString *)countryCode
                        countryName:(NSString *)countryName
                           iso3Code:(NSString *)iso3Code
                             status:(NSString *)status
                          continent:(NSString *)continent
                             region:(NSString *)region
                    frenchShortName:(NSString *)frenchShortName NS_DESIGNATED_INITIALIZER;
- (instancetype)init NS_UNAVAILABLE;

@end

@interface RGReverseGeocodeResult : NSObject

@property (nonatomic, strong, readonly) RGCountryMetadata *country;
@property (nonatomic, copy, readonly) NSString *stateCode;
@property (nonatomic, copy, readonly) NSString *stateName;

- (instancetype)initWithCountry:(RGCountryMetadata *)country
                      stateCode:(NSString *)stateCode
                      stateName:(NSString *)stateName NS_DESIGNATED_INITIALIZER;
- (instancetype)init NS_UNAVAILABLE;

@end

@interface RGReverseGeocoder : NSObject

- (instancetype)init;
- (nullable NSString *)reverseGeocodeLatitude:(double)latitude longitude:(double)longitude;
- (nullable NSString *)reverseGeocodeStateLatitude:(double)latitude longitude:(double)longitude;
- (nullable RGReverseGeocodeResult *)reverseGeocodeResultLatitude:(double)latitude longitude:(double)longitude;
- (nullable RGCountryMetadata *)countryMetadataForCode:(NSString *)countryCode;
- (NSArray<RGCountryMetadata *> *)allCountryMetadata;
- (NSUInteger)countryCount;

@end

NS_ASSUME_NONNULL_END
