// swift-tools-version: 5.10

import PackageDescription

let package = Package(
    name: "ReverseGeocodeSDKConsumer",
    platforms: [
        .iOS(.v15),
    ],
    products: [
        .library(
            name: "ReverseGeocodeSDKConsumer",
            targets: ["ReverseGeocodeSDKConsumer"]
        ),
    ],
    targets: [
        .binaryTarget(
            name: "ReverseGeocodeSDK",
            path: "../../../outputs/ReverseGeocodeSDK.xcframework"
        ),
        .target(
            name: "ReverseGeocodeSDKConsumer",
            dependencies: ["ReverseGeocodeSDK"]
        ),
        .testTarget(
            name: "ReverseGeocodeSDKConsumerTests",
            dependencies: ["ReverseGeocodeSDKConsumer"]
        ),
    ]
)
