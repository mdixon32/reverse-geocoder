# Polygon Viewer

This viewer renders local GeoJSON or `geo_shape` polygon files so you can inspect country outlines in a browser.

## Run locally

From the repo root:

```sh
python3 -m http.server 8000
```

Then open:

```text
http://127.0.0.1:8000/SDK/examples/polygon-viewer/
```

By default, the viewer looks for files in:

```text
/work/polygon-data/<ISO2>.json
```

You can override that with a `root` query parameter:

```text
http://127.0.0.1:8000/SDK/examples/polygon-viewer/?root=/my-polygons
```

## Features

- Search by country name, ISO2, or ISO3
- Load local country geometry JSON by ISO2 code
- Fit-to-bounds for the selected geometry
- Pan, wheel zoom, and double-click fit
- Bounding box, point count, and ring count metadata

## Notes

- The viewer uses a simple equirectangular projection for inspection, not production cartography.
- The loader accepts `Feature`, `FeatureCollection`, `Polygon`, `MultiPolygon`, or a wrapper object containing `geo_shape`.
