#!/bin/zsh

set -euo pipefail
setopt null_glob
setopt typeset_silent

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="$ROOT_DIR/build/ios-xcframework"
OUTPUT_DIR="${1:-$ROOT_DIR/../outputs}"
XCFRAMEWORK_PATH="$OUTPUT_DIR/ReverseGeocodeSDK.xcframework"
HEADERS_DIR="$BUILD_DIR/headers"
MODULES_DIR="$HEADERS_DIR/Modules"

mkdir -p "$BUILD_DIR/device" "$BUILD_DIR/simulator" "$HEADERS_DIR" "$MODULES_DIR" "$OUTPUT_DIR"
rm -rf "$XCFRAMEWORK_PATH"
rm -f "$BUILD_DIR/device/"*.o "$BUILD_DIR/simulator/"*.o

cp "$ROOT_DIR/ios/RGReverseGeocoder.h" "$HEADERS_DIR/"
cp "$ROOT_DIR/ios/ReverseGeocodeSDK.h" "$HEADERS_DIR/"
cp "$ROOT_DIR/ios/module.modulemap" "$MODULES_DIR/"

CORE_SOURCES=(
  "$ROOT_DIR/src/BoundingBox.cpp"
  "$ROOT_DIR/src/CountryBoundaryStore.cpp"
  "$ROOT_DIR/src/EncodedCountryBoundary.cpp"
  "$ROOT_DIR/src/Geometry.cpp"
  "$ROOT_DIR/src/ReverseGeocoder.cpp"
  "$ROOT_DIR/src/c_api.cpp"
  "$ROOT_DIR/data/DefaultCountryDataProvider.cpp"
  "$ROOT_DIR/data/DefaultUSStateDataProvider.cpp"
  "$ROOT_DIR/data/generated/CountryRegistry.cpp"
  "$ROOT_DIR/data/us_states/generated/USStateRegistry.cpp"
)

COUNTRY_SOURCES=()
for country_source in "$ROOT_DIR"/data/countries/*.cpp; do
  if [[ "$(basename "$country_source")" == "CountryBoundary.template.cpp" ]]; then
    continue
  fi
  COUNTRY_SOURCES+=("$country_source")
done
US_STATE_SOURCES=()
for state_source in "$ROOT_DIR"/data/us_states/*.cpp; do
  US_STATE_SOURCES+=("$state_source")
done
COMMON_SOURCES=("${CORE_SOURCES[@]}" "${COUNTRY_SOURCES[@]}" "${US_STATE_SOURCES[@]}" "$ROOT_DIR/ios/RGReverseGeocoder.mm")
COMMON_FLAGS=(
  -std=c++17
  -Os
  -fno-rtti
  -I"$ROOT_DIR/include"
  -I"$ROOT_DIR/data"
  -I"$ROOT_DIR/ios"
)

DEVICE_SDK="$(xcrun --sdk iphoneos --show-sdk-path)"
SIMULATOR_SDK="$(xcrun --sdk iphonesimulator --show-sdk-path)"

build_objects() {
  local sdk_path="$1"
  local target="$2"
  local min_flag="$3"
  local out_dir="$4"

  for source in "${COMMON_SOURCES[@]}"; do
    local base_name
    base_name="$(basename "$source")"
    local object_path="$out_dir/${base_name%.*}.o"
    local extra_flags=()

    if [[ "$source" == *.mm ]]; then
      extra_flags=(-fobjc-arc)
    fi

    xcrun clang++ "${COMMON_FLAGS[@]}" "${extra_flags[@]}" \
      -isysroot "$sdk_path" \
      "$min_flag" \
      -target "$target" \
      -c "$source" \
      -o "$object_path"
  done
}

build_objects "$DEVICE_SDK" "arm64-apple-ios15.0" "-miphoneos-version-min=15.0" "$BUILD_DIR/device"
/usr/bin/libtool -static -o "$BUILD_DIR/device/libReverseGeocodeSDK.a" "$BUILD_DIR/device/"*.o
xcrun strip -S -x "$BUILD_DIR/device/libReverseGeocodeSDK.a"

build_objects "$SIMULATOR_SDK" "arm64-apple-ios15.0-simulator" "-mios-simulator-version-min=15.0" "$BUILD_DIR/simulator"
/usr/bin/libtool -static -o "$BUILD_DIR/simulator/libReverseGeocodeSDK.a" "$BUILD_DIR/simulator/"*.o
xcrun strip -S -x "$BUILD_DIR/simulator/libReverseGeocodeSDK.a"

xcodebuild -create-xcframework \
  -library "$BUILD_DIR/device/libReverseGeocodeSDK.a" -headers "$HEADERS_DIR" \
  -library "$BUILD_DIR/simulator/libReverseGeocodeSDK.a" -headers "$HEADERS_DIR" \
  -output "$XCFRAMEWORK_PATH"

du -sh "$XCFRAMEWORK_PATH"
