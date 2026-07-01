# iOS Wrapper

`RGReverseGeocoder` is a thin Objective-C++ wrapper over the C++ core.

App-facing use:

```objc
RGReverseGeocoder *geocoder = [[RGReverseGeocoder alloc] init];
NSString *countryCode = [geocoder reverseGeocodeLatitude:51.5072 longitude:-0.1276];
```

Once you compile in real country data, this can remain the only method your app calls.
