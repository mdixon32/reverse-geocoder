import MapKit
import ReverseGeocodeSDK
import SwiftUI

struct ReverseGeocodeMapView: View {
    @StateObject private var model = ReverseGeocodeMapModel()

    var body: some View {
        ZStack(alignment: .top) {
            TappableWorldMapView { coordinate in
                model.resolve(coordinate: coordinate)
            }
            .ignoresSafeArea()

            if let countryName = model.resolvedCountryName {
                Text(countryName)
                    .font(.headline)
                    .padding(.horizontal, 16)
                    .padding(.vertical, 10)
                    .background(.ultraThinMaterial, in: Capsule())
                    .shadow(radius: 8, y: 4)
                    .padding(.top, 20)
                    .transition(.opacity.combined(with: .move(edge: .top)))
            }
        }
        .animation(.easeInOut(duration: 0.2), value: model.resolvedCountryName)
    }
}

@MainActor
final class ReverseGeocodeMapModel: ObservableObject {
    @Published var resolvedCountryName: String?

    private let geocoder = RGReverseGeocoder()
    private var hideWorkItem: DispatchWorkItem?

    func resolve(coordinate: CLLocationCoordinate2D) {
        let result = geocoder.reverseGeocodeResultLatitude(
            coordinate.latitude,
            longitude: coordinate.longitude
        )
        resolvedCountryName = result?.country.countryName ?? "No country found"

        hideWorkItem?.cancel()

        let workItem = DispatchWorkItem { [weak self] in
            self?.resolvedCountryName = nil
        }

        hideWorkItem = workItem
        DispatchQueue.main.asyncAfter(deadline: .now() + 3, execute: workItem)
    }
}

private struct TappableWorldMapView: UIViewRepresentable {
    let onTapCoordinate: (CLLocationCoordinate2D) -> Void

    func makeCoordinator() -> Coordinator {
        Coordinator(onTapCoordinate: onTapCoordinate)
    }

    func makeUIView(context: Context) -> MKMapView {
        let mapView = MKMapView(frame: .zero)
        mapView.delegate = context.coordinator
        mapView.pointOfInterestFilter = .excludingAll
        mapView.showsCompass = false
        mapView.showsScale = false

        let region = MKCoordinateRegion(
            center: CLLocationCoordinate2D(latitude: 20, longitude: 0),
            span: MKCoordinateSpan(latitudeDelta: 140, longitudeDelta: 140)
        )
        mapView.setRegion(region, animated: false)

        let tapGesture = UITapGestureRecognizer(
            target: context.coordinator,
            action: #selector(Coordinator.handleTap(_:))
        )
        tapGesture.cancelsTouchesInView = false
        mapView.addGestureRecognizer(tapGesture)

        return mapView
    }

    func updateUIView(_ mapView: MKMapView, context: Context) {
        context.coordinator.onTapCoordinate = onTapCoordinate
    }

    final class Coordinator: NSObject, MKMapViewDelegate {
        var onTapCoordinate: (CLLocationCoordinate2D) -> Void

        init(onTapCoordinate: @escaping (CLLocationCoordinate2D) -> Void) {
            self.onTapCoordinate = onTapCoordinate
        }

        @objc func handleTap(_ gestureRecognizer: UITapGestureRecognizer) {
            guard
                let mapView = gestureRecognizer.view as? MKMapView,
                gestureRecognizer.state == .ended
            else {
                return
            }

            let point = gestureRecognizer.location(in: mapView)
            let coordinate = mapView.convert(point, toCoordinateFrom: mapView)
            onTapCoordinate(coordinate)
        }
    }
}
