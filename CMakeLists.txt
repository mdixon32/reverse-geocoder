cmake_minimum_required(VERSION 3.20)

project(ReverseGeocodeSDK VERSION 0.1.0 LANGUAGES CXX OBJCXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB SDK_COUNTRY_SOURCES CONFIGURE_DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/data/countries/*.cpp
)
file(GLOB SDK_US_STATE_SOURCES CONFIGURE_DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/data/us_states/*.cpp
)

list(FILTER SDK_COUNTRY_SOURCES EXCLUDE REGEX ".*/CountryBoundary\\.template\\.cpp$")

add_library(reverse_geocode_core
  src/BoundingBox.cpp
  src/CountryBoundaryStore.cpp
  src/EncodedCountryBoundary.cpp
  src/Geometry.cpp
  src/ReverseGeocoder.cpp
  src/c_api.cpp
  data/DefaultCountryDataProvider.cpp
  data/DefaultUSStateDataProvider.cpp
  data/generated/CountryRegistry.cpp
  data/us_states/generated/USStateRegistry.cpp
  ${SDK_COUNTRY_SOURCES}
  ${SDK_US_STATE_SOURCES}
)

target_include_directories(reverse_geocode_core
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(reverse_geocode_core PUBLIC cxx_std_17)
target_compile_options(reverse_geocode_core PRIVATE -Os -fno-rtti)

add_library(reverse_geocode_ios
  ios/RGReverseGeocoder.mm
)

target_link_libraries(reverse_geocode_ios PUBLIC reverse_geocode_core)
target_include_directories(reverse_geocode_ios PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/ios)
target_compile_options(reverse_geocode_ios PRIVATE -Os -fno-rtti)
set_target_properties(reverse_geocode_ios PROPERTIES
  FRAMEWORK FALSE
  XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC YES
)

add_library(reverse_geocode_android
  android/jni/ReverseGeocoderJNI.cpp
)

target_link_libraries(reverse_geocode_android PUBLIC reverse_geocode_core)
target_include_directories(reverse_geocode_android PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/android/include)
target_compile_options(reverse_geocode_android PRIVATE -Os -fno-rtti)

option(REVERSE_GEOCODE_BUILD_TESTS "Build SDK tests" ON)

if(REVERSE_GEOCODE_BUILD_TESTS)
  enable_testing()

  add_executable(reverse_geocode_tests
    tests/ReverseGeocoderTests.cpp
  )

  target_link_libraries(reverse_geocode_tests PRIVATE reverse_geocode_core)
  add_test(NAME reverse_geocode_tests COMMAND reverse_geocode_tests)
endif()
