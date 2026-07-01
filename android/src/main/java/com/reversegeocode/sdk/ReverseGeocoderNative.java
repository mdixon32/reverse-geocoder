package com.reversegeocode.sdk;

final class ReverseGeocoderNative {
    static {
        System.loadLibrary("reversegeocode");
    }

    private long handle = nativeCreate();

    String reverseGeocode(double latitude, double longitude) {
        return nativeReverseGeocode(handle, latitude, longitude);
    }

    String reverseGeocodeState(double latitude, double longitude) {
        return nativeReverseGeocodeState(handle, latitude, longitude);
    }

    int countryCount() {
        return nativeCountryCount(handle);
    }

    void close() {
        if (handle != 0L) {
            nativeDestroy(handle);
            handle = 0L;
        }
    }

    private native long nativeCreate();

    private native void nativeDestroy(long handle);

    private native String nativeReverseGeocode(long handle, double latitude, double longitude);

    private native String nativeReverseGeocodeState(long handle, double latitude, double longitude);

    private native int nativeCountryCount(long handle);
}
