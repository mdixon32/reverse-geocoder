#include <jni.h>

#include <memory>
#include <optional>
#include <string>

#include "rg/ReverseGeocoder.h"

namespace {

jlong toHandle(std::unique_ptr<rg::ReverseGeocoder> geocoder) {
  return reinterpret_cast<jlong>(geocoder.release());
}

rg::ReverseGeocoder* fromHandle(jlong handle) {
  return reinterpret_cast<rg::ReverseGeocoder*>(handle);
}

}  // namespace

extern "C" JNIEXPORT jlong JNICALL
Java_com_reversegeocode_sdk_ReverseGeocoderNative_nativeCreate(JNIEnv*, jobject) {
  return toHandle(std::make_unique<rg::ReverseGeocoder>());
}

extern "C" JNIEXPORT void JNICALL
Java_com_reversegeocode_sdk_ReverseGeocoderNative_nativeDestroy(JNIEnv*, jobject, jlong handle) {
  delete fromHandle(handle);
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_reversegeocode_sdk_ReverseGeocoderNative_nativeReverseGeocode(JNIEnv* env,
                                                                       jobject,
                                                                       jlong handle,
                                                                       jdouble latitude,
                                                                       jdouble longitude) {
  rg::ReverseGeocoder* geocoder = fromHandle(handle);
  if (geocoder == nullptr) {
    return nullptr;
  }

  const std::optional<std::string> result = geocoder->reverseGeocode(latitude, longitude);
  if (!result.has_value()) {
    return nullptr;
  }

  return env->NewStringUTF(result->c_str());
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_reversegeocode_sdk_ReverseGeocoderNative_nativeReverseGeocodeState(JNIEnv* env,
                                                                            jobject,
                                                                            jlong handle,
                                                                            jdouble latitude,
                                                                            jdouble longitude) {
  rg::ReverseGeocoder* geocoder = fromHandle(handle);
  if (geocoder == nullptr) {
    return nullptr;
  }

  const std::optional<std::string> result = geocoder->reverseGeocodeState(latitude, longitude);
  if (!result.has_value()) {
    return nullptr;
  }

  return env->NewStringUTF(result->c_str());
}

extern "C" JNIEXPORT jint JNICALL
Java_com_reversegeocode_sdk_ReverseGeocoderNative_nativeCountryCount(JNIEnv*,
                                                                     jobject,
                                                                     jlong handle) {
  rg::ReverseGeocoder* geocoder = fromHandle(handle);
  if (geocoder == nullptr) {
    return 0;
  }

  return static_cast<jint>(geocoder->countryCount());
}
