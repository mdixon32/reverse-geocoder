#pragma once

// Placeholder include path for non-Android local builds.
// Replace this with the real Android NDK JNI include path in your Android build.

typedef int jint;
typedef long long jlong;
typedef double jdouble;
typedef void* jobject;
typedef void* jstring;

struct JNIEnv {
  jstring NewStringUTF(const char*);
};

#define JNIEXPORT
#define JNICALL
