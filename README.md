# ReverseGeocode SDK

Portable reverse-geocoding SDK scaffold for iOS and Android apps.

## Shape

- C++ core contains geometry, boundary storage, and `reverseGeocode`.
- Country boundaries are compiled into the SDK through provider files in `data/`.
- iOS gets an Objective-C++ wrapper.
- Android gets a JNI bridge and an Android library package consumable from Kotlin or Java.

## Public entry points

- C++: `rg::ReverseGeocoder::reverseGeocode(latitude, longitude)`
- C++ detailed result: `rg::ReverseGeocoder::reverseGeocodeResult(latitude, longitude)`
- C++ metadata lookup: `rg::ReverseGeocoder::countryMetadata(countryCode)`
- C++ metadata catalog: `rg::ReverseGeocoder::allCountryMetadata()`
- C: `rg_reverse_geocode(...)`
- iOS: `-[RGReverseGeocoder reverseGeocodeLatitude:longitude:]`
- Android/Kotlin: `ReverseGeocoder.reverseGeocode(latitude, longitude)`
- Android/Kotlin: `ReverseGeocoder.reverseGeocodeState(latitude, longitude)`

## Build

```bash
cmake -S SDK -B SDK/build
cmake --build SDK/build
ctest --test-dir SDK/build
```

Android AAR:

```bash
zsh SDK/scripts/build_android_aar.sh /Users/martindixon/Documents/Codex/2026-06-29/i/outputs
```

## Notes

- The app-facing API can remain a single `reverseGeocode` method if you compile all boundary data into the provider.
- If the dataset becomes too large for a single static binary blob, the next step is usually generated regional shards plus a compile-time index, still behind the same API.
