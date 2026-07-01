cmake_minimum_required(VERSION 3.20)

project(ReverseGeocodeAndroid LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SDK_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/..)

file(GLOB SDK_COUNTRY_SOURCES CONFIGURE_DEPENDS
  ${SDK_ROOT}/data/countries/*.cpp
)
file(GLOB SDK_US_STATE_SOURCES CONFIGURE_DEPENDS
  ${SDK_ROOT}/data/us_states/*.cpp
)

list(FILTER SDK_COUNTRY_SOURCES EXCLUDE REGEX ".*/CountryBoundary\\.template\\.cpp$")

add_library(reversegeocode SHARED
  ${SDK_ROOT}/src/BoundingBox.cpp
  ${SDK_ROOT}/src/CountryBoundaryStore.cpp
  ${SDK_ROOT}/src/EncodedCountryBoundary.cpp
  ${SDK_ROOT}/src/Geometry.cpp
  ${SDK_ROOT}/src/ReverseGeocoder.cpp
  ${SDK_ROOT}/src/c_api.cpp
  ${SDK_ROOT}/data/DefaultCountryDataProvider.cpp
  ${SDK_ROOT}/data/DefaultUSStateDataProvider.cpp
  ${SDK_ROOT}/data/generated/CountryRegistry.cpp
  ${SDK_ROOT}/data/us_states/generated/USStateRegistry.cpp
  ${SDK_COUNTRY_SOURCES}
  ${SDK_US_STATE_SOURCES}
  ${CMAKE_CURRENT_SOURCE_DIR}/jni/ReverseGeocoderJNI.cpp
)

target_include_directories(reversegeocode
  PRIVATE
    ${SDK_ROOT}/include
    ${SDK_ROOT}/data
)

target_compile_options(reversegeocode PRIVATE -Os -fno-rtti)
