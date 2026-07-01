#!/bin/zsh

set -euo pipefail
setopt null_glob

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
ANDROID_DIR="$ROOT_DIR/android"
BUILD_DIR="$ROOT_DIR/build/android-aar"
OUTPUT_DIR="${1:-$ROOT_DIR/../outputs}"
AAR_PATH="$OUTPUT_DIR/ReverseGeocodeSDK-android.aar"

ANDROID_SDK_ROOT="${ANDROID_SDK_ROOT:-$HOME/Library/Android/sdk}"
if [[ -z "${ANDROID_NDK_ROOT:-}" ]]; then
  ANDROID_NDK_ROOT="$(find "$ANDROID_SDK_ROOT/ndk" -maxdepth 1 -mindepth 1 -type d | sort | tail -1)"
fi
CMAKE_BIN="$(find "$ANDROID_SDK_ROOT/cmake" -maxdepth 3 -type f -name cmake | sort | tail -1)"
ANDROID_JAR="$(find "$ANDROID_SDK_ROOT/platforms" -maxdepth 2 -type f -name android.jar | sort | tail -1)"

ABIS=(
  arm64-v8a
  armeabi-v7a
  x86_64
)

mkdir -p "$BUILD_DIR" "$OUTPUT_DIR"
rm -rf "$BUILD_DIR/package"
rm -f "$AAR_PATH"

if [[ -z "$ANDROID_NDK_ROOT" || ! -d "$ANDROID_NDK_ROOT" ]]; then
  echo "Could not find an installed Android NDK under $ANDROID_SDK_ROOT/ndk" >&2
  exit 1
fi

if [[ -z "$CMAKE_BIN" || ! -x "$CMAKE_BIN" ]]; then
  echo "Could not find Android SDK CMake under $ANDROID_SDK_ROOT/cmake" >&2
  exit 1
fi

if [[ -z "$ANDROID_JAR" || ! -f "$ANDROID_JAR" ]]; then
  echo "Could not find android.jar under $ANDROID_SDK_ROOT/platforms" >&2
  exit 1
fi

for abi in "${ABIS[@]}"; do
  ABI_BUILD_DIR="$BUILD_DIR/$abi"
  rm -rf "$ABI_BUILD_DIR"

  "$CMAKE_BIN" -S "$ANDROID_DIR" -B "$ABI_BUILD_DIR" -G Ninja \
    -DCMAKE_TOOLCHAIN_FILE="$ANDROID_NDK_ROOT/build/cmake/android.toolchain.cmake" \
    -DANDROID_ABI="$abi" \
    -DANDROID_PLATFORM=24 \
    -DANDROID_STL=c++_static \
    -DCMAKE_BUILD_TYPE=Release

  "$CMAKE_BIN" --build "$ABI_BUILD_DIR"
done

JAVA_CLASSES_DIR="$BUILD_DIR/java-classes"
CLASSES_JAR_DIR="$BUILD_DIR/classes-jar"
PACKAGE_DIR="$BUILD_DIR/package"

rm -rf "$JAVA_CLASSES_DIR" "$CLASSES_JAR_DIR" "$PACKAGE_DIR"
mkdir -p "$JAVA_CLASSES_DIR" "$CLASSES_JAR_DIR" "$PACKAGE_DIR/jni"

javac \
  -source 8 \
  -target 8 \
  -classpath "$ANDROID_JAR" \
  -d "$JAVA_CLASSES_DIR" \
  $(find "$ANDROID_DIR/src/main/java" -name '*.java' | sort)

jar cf "$CLASSES_JAR_DIR/classes.jar" -C "$JAVA_CLASSES_DIR" .

cp "$ANDROID_DIR/src/main/AndroidManifest.xml" "$PACKAGE_DIR/AndroidManifest.xml"
cp "$CLASSES_JAR_DIR/classes.jar" "$PACKAGE_DIR/classes.jar"

for abi in "${ABIS[@]}"; do
  mkdir -p "$PACKAGE_DIR/jni/$abi"
  cp "$BUILD_DIR/$abi/libreversegeocode.so" "$PACKAGE_DIR/jni/$abi/"
done

(
  cd "$PACKAGE_DIR"
  zip -qr "$AAR_PATH" AndroidManifest.xml classes.jar jni
)

du -sh "$AAR_PATH"
