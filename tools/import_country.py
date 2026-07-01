#!/usr/bin/env python3

from __future__ import annotations

import argparse
import json
import re
from pathlib import Path
from typing import Iterable

COORDINATE_DECIMALS = 5
COORDINATE_SCALE = 100_000


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Import country geoshape data into generated C++ source for the ReverseGeocode SDK."
    )
    parser.add_argument("--iso-country-code", "--iso", dest="iso_country_code", required=True)
    parser.add_argument("--country-name", "--name", dest="country_name", required=True)
    parser.add_argument("--iso3-code", default="")
    parser.add_argument("--status", default="")
    parser.add_argument("--continent", default="")
    parser.add_argument("--region", default="")
    parser.add_argument("--french-short-name", default="")

    geoshape_group = parser.add_mutually_exclusive_group(required=True)
    geoshape_group.add_argument("--geoshape-file", type=Path)
    geoshape_group.add_argument("--geoshape-json")

    parser.add_argument(
        "--project-root",
        type=Path,
        default=Path(__file__).resolve().parents[1],
        help="SDK root path. Defaults to the current SDK directory.",
    )
    return parser.parse_args()


def load_geoshape(args: argparse.Namespace) -> object:
    if args.geoshape_file is not None:
        return json.loads(args.geoshape_file.read_text(encoding="utf-8"))

    return json.loads(args.geoshape_json)


def normalize_ring(ring: list[list[float]]) -> list[tuple[float, float]]:
    points = [
        (round(float(lon), COORDINATE_DECIMALS), round(float(lat), COORDINATE_DECIMALS))
        for lon, lat in ring
    ]
    if not points:
        raise ValueError("ring must not be empty")

    if points[0] != points[-1]:
        points.append(points[0])

    if len(points) < 4:
        raise ValueError("ring must contain at least four points including closure")

    return points


def geometry_to_polygons(geometry: dict) -> list[list[list[tuple[float, float]]]]:
    geometry_type = geometry.get("type")
    coordinates = geometry.get("coordinates")

    if geometry_type == "Polygon":
        return [[normalize_ring(ring) for ring in coordinates]]

    if geometry_type == "MultiPolygon":
        return [[normalize_ring(ring) for ring in polygon] for polygon in coordinates]

    raise ValueError(f"unsupported geometry type: {geometry_type}")


def extract_polygons(geoshape: object) -> list[list[list[tuple[float, float]]]]:
    if not isinstance(geoshape, dict):
        raise ValueError("geoshape must be a JSON object")

    shape_type = geoshape.get("type")

    if shape_type in {"Polygon", "MultiPolygon"}:
        return geometry_to_polygons(geoshape)

    if shape_type == "Feature":
        return geometry_to_polygons(geoshape["geometry"])

    if shape_type == "FeatureCollection":
        polygons: list[list[list[tuple[float, float]]]] = []
        for feature in geoshape.get("features", []):
          polygons.extend(geometry_to_polygons(feature["geometry"]))
        return polygons

    raise ValueError(f"unsupported geoshape type: {shape_type}")


def slugify(value: str) -> str:
    slug = re.sub(r"[^A-Za-z0-9]+", "_", value).strip("_")
    return slug or "Country"


def function_name(iso_country_code: str) -> str:
    return f"makeCountryBoundary{iso_country_code.upper()}"


def escape_cpp_string(value: str) -> str:
    return value.replace("\\", "\\\\").replace('"', '\\"')


def encode_point(point: tuple[float, float]) -> tuple[int, int]:
    longitude, latitude = point
    return round(longitude * COORDINATE_SCALE), round(latitude * COORDINATE_SCALE)


def zigzag_encode(value: int) -> int:
    return (value << 1) ^ (value >> 31)


def encode_varuint(value: int) -> list[int]:
    encoded: list[int] = []
    while True:
        next_byte = value & 0x7F
        value >>= 7
        if value:
            encoded.append(next_byte | 0x80)
        else:
            encoded.append(next_byte)
            return encoded


def encode_signed_varint(value: int) -> list[int]:
    return encode_varuint(zigzag_encode(value))


def encode_ring_bytes(ring: list[tuple[float, float]]) -> list[int]:
    stored_ring = ring[:-1] if ring and ring[0] == ring[-1] else ring
    if len(stored_ring) < 3:
        raise ValueError("ring must contain at least three unique points")

    encoded: list[int] = []
    encoded.extend(encode_varuint(len(stored_ring)))

    previous_longitude = 0
    previous_latitude = 0
    for index, point in enumerate(stored_ring):
        longitude_fixed, latitude_fixed = encode_point(point)
        if index == 0:
            encoded.extend(encode_signed_varint(longitude_fixed))
            encoded.extend(encode_signed_varint(latitude_fixed))
        else:
            encoded.extend(encode_signed_varint(longitude_fixed - previous_longitude))
            encoded.extend(encode_signed_varint(latitude_fixed - previous_latitude))

        previous_longitude = longitude_fixed
        previous_latitude = latitude_fixed

    return encoded


def encode_polygon_bytes(polygon: list[list[tuple[float, float]]]) -> list[int]:
    encoded = encode_varuint(len(polygon))
    for ring in polygon:
        encoded.extend(encode_ring_bytes(ring))
    return encoded


def render_byte_array(values: list[int], indent: str = "    ", line_width: int = 16) -> list[str]:
    if not values:
        return [f"{indent}0x00,"]

    lines: list[str] = []
    for index in range(0, len(values), line_width):
        chunk = values[index:index + line_width]
        rendered_chunk = ", ".join(f"0x{value:02x}" for value in chunk)
        lines.append(f"{indent}{rendered_chunk},")
    return lines


def render_country_source(iso_country_code: str,
                          country_name: str,
                          metadata: dict[str, str],
                          polygons: list[list[list[tuple[float, float]]]]) -> str:
    polygon_bytes: list[int] = []
    for polygon in polygons:
        polygon_bytes.extend(encode_polygon_bytes(polygon))

    lines = [
        '#include "rg/EncodedCountryBoundary.h"',
        "",
        "namespace rg {",
        "namespace {",
        "",
        "const std::uint8_t kPolygonData[] = {",
    ]
    lines.extend(render_byte_array(polygon_bytes))
    lines.extend(
        [
            "};",
            "",
            "}  // namespace",
            "",
            f"CountryBoundary {function_name(iso_country_code)}() {{",
            "  return decodeCountryBoundary({",
            f'      "{iso_country_code.upper()}",',
            f'      "{escape_cpp_string(country_name)}",',
            f'      "{escape_cpp_string(metadata["iso3Code"])}",',
            f'      "{escape_cpp_string(metadata["status"])}",',
            f'      "{escape_cpp_string(metadata["continent"])}",',
            f'      "{escape_cpp_string(metadata["region"])}",',
            f'      "{escape_cpp_string(metadata["frenchShortName"])}",',
            "      kPolygonData,",
            "      sizeof(kPolygonData),",
            f"      {len(polygons)},",
            "  });",
            "}",
            "",
            "}  // namespace rg",
            "",
        ]
    )

    return "\n".join(lines)


def load_manifest(manifest_path: Path) -> list[dict[str, str]]:
    if not manifest_path.exists():
        return []
    return json.loads(manifest_path.read_text(encoding="utf-8"))


def write_manifest(manifest_path: Path, entries: list[dict[str, str]]) -> None:
    manifest_path.write_text(f"{json.dumps(entries, indent=2)}\n", encoding="utf-8")


def render_registry(entries: Iterable[dict[str, str]]) -> str:
    entries = list(entries)
    lines = [
        '#include "CountryRegistry.h"',
        "",
        "namespace rg {",
        "",
    ]

    for entry in entries:
        lines.append(f"CountryBoundary {entry['function']}();")

    lines.extend(
        [
            "",
            "std::vector<CountryBoundary> loadGeneratedCountries() {",
            "  return {",
        ]
    )

    for entry in entries:
        lines.append(f"      {entry['function']}(),")

    lines.extend(
        [
            "  };",
            "}",
            "",
            "}  // namespace rg",
            "",
        ]
    )

    return "\n".join(lines)


def ensure_directories(*paths: Path) -> None:
    for path in paths:
        path.mkdir(parents=True, exist_ok=True)


def main() -> None:
    args = parse_args()
    iso_country_code = args.iso_country_code.strip().upper()
    country_name = args.country_name.strip()
    metadata = {
        "iso3Code": args.iso3_code.strip().upper(),
        "status": args.status.strip(),
        "continent": args.continent.strip(),
        "region": args.region.strip(),
        "frenchShortName": args.french_short_name.strip(),
    }

    if not re.fullmatch(r"[A-Z]{2}", iso_country_code):
        raise ValueError("isoCountryCode must be a two-letter ISO code")

    if not country_name:
        raise ValueError("countryName must not be empty")

    project_root = args.project_root.resolve()
    countries_dir = project_root / "data" / "countries"
    generated_dir = project_root / "data" / "generated"
    manifest_path = countries_dir / "manifest.json"
    registry_path = generated_dir / "CountryRegistry.cpp"

    ensure_directories(countries_dir, generated_dir)

    geoshape = load_geoshape(args)
    polygons = extract_polygons(geoshape)

    if not polygons:
        raise ValueError("geoshape did not contain any polygons")

    file_name = f"{iso_country_code}_{slugify(country_name)}.cpp"
    output_path = countries_dir / file_name
    existing_entries = load_manifest(manifest_path)
    previous_entry = next(
        (entry for entry in existing_entries if entry["isoCountryCode"] == iso_country_code),
        None,
    )

    if previous_entry is not None and previous_entry["file"] != file_name:
        previous_path = countries_dir / previous_entry["file"]
        if previous_path.exists():
            previous_path.unlink()

    output_path.write_text(
        render_country_source(iso_country_code, country_name, metadata, polygons),
        encoding="utf-8",
    )

    manifest = [entry for entry in existing_entries if entry["isoCountryCode"] != iso_country_code]

    manifest.append(
        {
            "isoCountryCode": iso_country_code,
            "countryName": country_name,
            "iso3Code": metadata["iso3Code"],
            "status": metadata["status"],
            "continent": metadata["continent"],
            "region": metadata["region"],
            "frenchShortName": metadata["frenchShortName"],
            "file": file_name,
            "function": function_name(iso_country_code),
        }
    )
    manifest.sort(key=lambda entry: entry["isoCountryCode"])
    write_manifest(manifest_path, manifest)
    registry_path.write_text(render_registry(manifest), encoding="utf-8")

    print(f"Imported {country_name} ({iso_country_code})")
    print(f"Wrote {output_path}")
    print(f"Updated {registry_path}")


if __name__ == "__main__":
    main()
