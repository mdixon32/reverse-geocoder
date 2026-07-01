# Android Library

The Android side is split into:

- `jni/ReverseGeocoderJNI.cpp` for the native bridge
- `src/main/java/com/reversegeocode/sdk/...` for the Android-facing API
- `CMakeLists.txt` for the NDK shared library build

Build the consumable AAR with:

```bash
zsh SDK/scripts/build_android_aar.sh /Users/martindixon/Documents/Codex/2026-06-29/i/outputs
```

That writes:

- `outputs/ReverseGeocodeSDK-android.aar`

Kotlin app use:

```kotlin
import com.reversegeocode.sdk.ReverseGeocoder

ReverseGeocoder().use { geocoder ->
    val countryCode = geocoder.reverseGeocode(51.5072, -0.1276)
    val stateCode = geocoder.reverseGeocodeState(37.7749, -122.4194)
}
```

Consumer app wiring with a local file dependency:

```kotlin
dependencies {
    implementation(files("libs/ReverseGeocodeSDK-android.aar"))
}
```
