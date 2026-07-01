package com.reversegeocode.sdk;

import java.io.Closeable;

public final class ReverseGeocoder implements Closeable {
    private final ReverseGeocoderNative nativeBridge = new ReverseGeocoderNative();

    public String reverseGeocode(double latitude, double longitude) {
        return nativeBridge.reverseGeocode(latitude, longitude);
    }

    public String reverseGeocodeState(double latitude, double longitude) {
        return nativeBridge.reverseGeocodeState(latitude, longitude);
    }

    public int countryCount() {
        return nativeBridge.countryCount();
    }

    @Override
    public void close() {
        nativeBridge.close();
    }
}
