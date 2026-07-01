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

## Add country boundary data

Use the importer in [tools/import_country.py](/Users/martindixon/Documents/Codex/2026-06-29/i/SDK/tools/import_country.py). It generates one `.cpp` file per country under `data/countries/` and refreshes the generated registry automatically.

If your source is the Opendatasoft world administrative boundaries dataset, use [tools/import_from_opendatasoft.py](/Users/martindixon/Documents/Codex/2026-06-29/i/SDK/tools/import_from_opendatasoft.py). It downloads the raw dataset record, stores the source metadata locally, and then invokes the core importer.

For U.S. state boundaries derived from the Opendatasoft county dataset, use [tools/import_us_states_from_counties.py](/Users/martindixon/Documents/Codex/2026-06-29/i/SDK/tools/import_us_states_from_counties.py). It dissolves county polygons into state polygons and writes generated sources under `data/us_states/`.

Use the template at [data/countries/CountryBoundary.template.cpp](/Users/martindixon/Documents/Codex/2026-06-29/i/SDK/data/countries/CountryBoundary.template.cpp).

Example:

```bash
python3 SDK/tools/import_country.py \
  --iso-country-code CA \
  --country-name Canada \
  --geoshape-file latlon-country-worker/data/source/countries.geojson
```

Opendatasoft example:

```bash
python3 SDK/tools/import_from_opendatasoft.py --iso-country-code FR

python3 SDK/tools/import_us_states_from_counties.py --state-code CA

python3 SDK/tools/import_us_states_from_counties.py --all --include-dc
```

Each country file should return an `rg::CountryBoundary` with:

- `countryCode`
- `countryName`
- optional metadata fields such as `iso3Code`, `status`, `continent`, `region`, and `frenchShortName`
- `bbox`
- `polygons`
- each polygon containing one outer ring and optional hole rings
- points expressed as `{longitude, latitude}`

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
