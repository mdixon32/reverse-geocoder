import XCTest
@testable import ReverseGeocodeSDKConsumer

final class ReverseGeocodeSDKConsumerTests: XCTestCase {
    func testReverseGeocodeParis() {
        XCTAssertEqual(SmokeHarness.reverseGeocode(latitude: 48.8566, longitude: 2.3522), "FR")
    }

    func testReverseGeocodeNewYork() {
        XCTAssertEqual(SmokeHarness.reverseGeocode(latitude: 40.7128, longitude: -74.0060), "US")
    }

    func testCountryMetadata() {
        XCTAssertEqual(SmokeHarness.countryName(for: "CA"), "Canada")
    }

    func testCountryCount() {
        XCTAssertEqual(SmokeHarness.countryCount(), 193)
    }
}
