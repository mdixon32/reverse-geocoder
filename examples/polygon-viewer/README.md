# Polygon Viewer

This viewer renders the raw Opendatasoft `geo_shape` source polygons used to build the SDK data, so you can inspect country outlines before or after import changes.

## Run locally

From the repo root:

```sh
python3 -m http.server 8000
```

Then open:

```text
http://127.0.0.1:8000/SDK/examples/polygon-viewer/
```

## Features

- Search by country name, ISO2, or ISO3
- Load raw country source JSON from `SDK/data/source/opendatasoft`
- Fit-to-bounds for the selected geometry
- Pan, wheel zoom, and double-click fit
- Bounding box, point count, and ring count metadata

## Notes

- The viewer uses a simple equirectangular projection for inspection, not production cartography.
- Countries with overseas territories will render all source polygons included in the raw dataset.
