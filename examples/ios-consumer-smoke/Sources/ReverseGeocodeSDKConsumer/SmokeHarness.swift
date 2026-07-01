import Foundation
import ReverseGeocodeSDK

public enum SmokeHarness {
    public static func reverseGeocode(latitude: Double, longitude: Double) -> String? {
        RGReverseGeocoder().reverseGeocodeLatitude(latitude, longitude: longitude)
    }

    public static func countryName(for countryCode: String) -> String? {
        RGReverseGeocoder().countryMetadata(forCode: countryCode)?.countryName
    }

    public static func countryCount() -> Int {
        Int(RGReverseGeocoder().countryCount())
    }
}
