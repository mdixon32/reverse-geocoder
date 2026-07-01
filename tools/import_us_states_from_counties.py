#!/usr/bin/env python3

from __future__ import annotations

import argparse
import json
import os
import re
import sys
import urllib.parse
import urllib.request
import urllib.error
from pathlib import Path
from typing import Iterable

COORDINATE_DECIMALS = 5
COORDINATE_SCALE = 100_000

DEFAULT_RECORDS_URL = (
    "https://public.opendatasoft.com/api/explore/v2.1/catalog/datasets/"
    "us-county-boundaries/records"
)
DEFAULT_STATE_STATUS = "US State"
DEFAULT_CONTINENT = "Americas"
DEFAULT_REGION = "Northern America"
DEFAULT_COUNTRY_CODE = "US"
DEFAULT_COUNTRY_NAME = "United States of America"
VALID_STATE_CODES = {
    "AL", "AK", "AZ", "AR", "CA", "CO", "CT", "DE", "FL", "GA",
    "HI", "ID", "IL", "IN", "IA", "KS", "KY", "LA", "ME", "MD",
    "MA", "MI", "MN", "MS", "MO", "MT", "NE", "NV", "NH", "NJ",
    "NM", "NY", "NC", "ND", "OH", "OK", "OR", "PA", "RI", "SC",
    "SD", "TN", "TX", "UT", "VT", "VA", "WA", "WV", "WI", "WY",
}


def ensure_shapely_runtime() -> None:
    vendor_dir = Path(__file__).resolve().parent / "vendor"
    sys.path.insert(0, str(vendor_dir))

    try:
        import shapely  # noqa: F401
    except Exception:
        if sys.executable != "/usr/bin/python3" and Path("/usr/bin/python3").exists():
            os.execv("/usr/bin/python3", ["/usr/bin/python3", __file__, *sys.argv[1:]])
        raise


ensure_shapely_runtime()

from shapely.geometry import MultiPolygon, shape, mapping  # noqa: E402
from shapely.ops import unary_union  # noqa: E402


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description=(
            "Fetch the Opendatasoft US county boundaries dataset, dissolve county polygons "
            "into state polygons, and generate C++ source files for US states."
        )
    )
    target_group = parser.add_mutually_exclusive_group(required=True)
    target_group.add_argument("--state-code", "--state", dest="state_code")
    target_group.add_argument("--all", action="store_true")
    parser.add_argument(
        "--include-dc",
        action="store_true",
        help="Include the District of Columbia when importing all states.",
    )
    parser.add_argument(
        "--include-territories",
        action="store_true",
        help="Include American Samoa, Guam, Northern Mariana Islands, Puerto Rico, and the U.S. Virgin Islands.",
    )
    parser.add_argument(
        "--dataset-url",
        default=DEFAULT_RECORDS_URL,
        help="Override the Opendatasoft records API URL if needed.",
    )
    parser.add_argument(
        "--project-root",
        type=Path,
        default=Path(__file__).resolve().parents[1],
        help="SDK root path. Defaults to the current SDK directory.",
    )
    return parser.parse_args()


def fetch_json(url: str) -> dict:
    with urllib.request.urlopen(url) as response:
        return json.load(response)


def slugify(value: str) -> str:
    slug = re.sub(r"[^A-Za-z0-9]+", "_", value).strip("_")
    return slug or "Boundary"


def function_name(state_code: str) -> str:
    return f"makeUSStateBoundary{state_code.upper()}"


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


def extract_polygons(geoshape: object) -> list[list[list[tuple[float, float]]]]:
    if not isinstance(geoshape, dict):
        raise ValueError("geoshape must be a JSON object")

    geometry = geoshape.get("geometry") if geoshape.get("type") == "Feature" else geoshape
    if not isinstance(geometry, dict):
        raise ValueError("geoshape must contain a geometry object")

    geometry_type = geometry.get("type")
    coordinates = geometry.get("coordinates")

    if geometry_type == "Polygon":
        return [[normalize_ring(ring) for ring in coordinates]]

    if geometry_type == "MultiPolygon":
        return [[normalize_ring(ring) for ring in polygon] for polygon in coordinates]

    raise ValueError(f"unsupported geometry type: {geometry_type}")


def render_boundary_source(boundary_code: str,
                           display_name: str,
                           metadata: dict[str, str],
                           polygons: list[list[list[tuple[float, float]]]],
                           generated_function_name: str) -> str:
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
            f"CountryBoundary {generated_function_name}() {{",
            "  return decodeCountryBoundary({",
            f'      "{escape_cpp_string(boundary_code)}",',
            f'      "{escape_cpp_string(display_name)}",',
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


def render_registry_header() -> str:
    return "\n".join(
        [
            "#pragma once",
            "",
            '#include "rg/Types.h"',
            "",
            "#include <vector>",
            "",
            "namespace rg {",
            "",
            "std::vector<CountryBoundary> loadGeneratedUSStates();",
            "",
            "}  // namespace rg",
            "",
        ]
    )


def render_registry(entries: Iterable[dict[str, str]]) -> str:
    entries = list(entries)
    lines = [
        '#include "USStateRegistry.h"',
        "",
        "namespace rg {",
        "",
    ]

    for entry in entries:
        lines.append(f"CountryBoundary {entry['function']}();")

    lines.extend(
        [
            "",
            "std::vector<CountryBoundary> loadGeneratedUSStates() {",
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


def fetch_available_states(dataset_url: str) -> list[dict[str, object]]:
    query = urllib.parse.urlencode(
        {
            "select": "statefp,stusab,state_name,count(*) as county_count",
            "group_by": "statefp,stusab,state_name",
            "order_by": "stusab",
            "limit": 100,
        }
    )
    payload = fetch_json(f"{dataset_url}?{query}")
    return payload.get("results", [])


def fetch_counties_for_state(dataset_url: str, state_code: str) -> list[dict]:
    records: list[dict] = []
    offset = 0
    page_size = 20

    while True:
        query_params = {
            "where": f'stusab="{state_code}"',
            "select": "statefp,stusab,state_name,name,namelsad,geoid,geo_shape",
            "order_by": "geoid",
            "limit": page_size,
            "offset": offset,
        }

        while True:
            query = urllib.parse.urlencode(query_params)
            try:
                payload = fetch_json(f"{dataset_url}?{query}")
                break
            except urllib.error.HTTPError as error:
                if error.code != 400 or page_size <= 5:
                    raise
                page_size = max(5, page_size // 2)
                query_params["limit"] = page_size
        results = payload.get("results", [])
        records.extend(results)

        if len(results) < page_size:
            break

        offset += page_size

    if not records:
        raise ValueError(f"no county records found for state code {state_code}")

    return records


def dissolve_counties_to_state_geoshape(county_records: list[dict]) -> dict:
    county_geometries = []
    for record in county_records:
        geoshape = record.get("geo_shape")
        if not isinstance(geoshape, dict):
            continue
        geometry = geoshape.get("geometry") if geoshape.get("type") == "Feature" else geoshape
        if not isinstance(geometry, dict):
            continue
        county_geometries.append(shape(geometry))

    if not county_geometries:
        raise ValueError("state did not contain any polygon geometries")

    dissolved = unary_union(county_geometries)
    if not dissolved.is_valid:
        dissolved = dissolved.buffer(0)

    if dissolved.is_empty:
        raise ValueError("dissolved state geometry was empty")

    if dissolved.geom_type == "Polygon":
        dissolved = MultiPolygon([dissolved])
    elif dissolved.geom_type == "GeometryCollection":
        polygon_parts = []
        for part in dissolved.geoms:
          if part.geom_type == "Polygon":
              polygon_parts.append(part)
          elif part.geom_type == "MultiPolygon":
              polygon_parts.extend(part.geoms)
        if not polygon_parts:
            raise ValueError("dissolved geometry collection did not contain polygon parts")
        dissolved = MultiPolygon(polygon_parts)
    elif dissolved.geom_type != "MultiPolygon":
        raise ValueError(f"unsupported dissolved geometry type: {dissolved.geom_type}")

    return {
        "type": "Feature",
        "geometry": mapping(dissolved),
        "properties": {},
    }


def save_state_source(project_root: Path, state_record: dict, counties: list[dict]) -> Path:
    source_dir = project_root / "data" / "source" / "opendatasoft" / "us_states"
    ensure_directories(source_dir)
    output_path = source_dir / f"{state_record['stusab']}.json"
    payload = {
        "sourceDataset": "us-county-boundaries",
        "statefp": state_record["statefp"],
        "stusab": state_record["stusab"],
        "state_name": state_record["state_name"],
        "county_count": len(counties),
        "county_names": [county.get("namelsad") or county.get("name") for county in counties],
    }
    output_path.write_text(f"{json.dumps(payload, indent=2)}\n", encoding="utf-8")
    return output_path


def save_state_geoshape(project_root: Path, state_code: str, geoshape: dict) -> Path:
    geoshape_dir = project_root / "data" / "source" / "opendatasoft" / "us_states" / "geoshapes"
    ensure_directories(geoshape_dir)
    output_path = geoshape_dir / f"{state_code}.geo.json"
    output_path.write_text(f"{json.dumps(geoshape)}\n", encoding="utf-8")
    return output_path


def import_state(project_root: Path, state_record: dict, counties: list[dict]) -> dict[str, str]:
    state_code = str(state_record["stusab"]).upper()
    state_name = str(state_record["state_name"]).strip()
    boundary_code = f"{DEFAULT_COUNTRY_CODE}-{state_code}"
    generated_function_name = function_name(state_code)
    geoshape = dissolve_counties_to_state_geoshape(counties)
    polygons = extract_polygons(geoshape)

    states_dir = project_root / "data" / "us_states"
    generated_dir = states_dir / "generated"
    manifest_path = states_dir / "manifest.json"
    registry_path = generated_dir / "USStateRegistry.cpp"
    registry_header_path = generated_dir / "USStateRegistry.h"
    ensure_directories(states_dir, generated_dir)

    save_state_source(project_root, state_record, counties)
    save_state_geoshape(project_root, state_code, geoshape)

    file_name = f"US_{state_code}_{slugify(state_name)}.cpp"
    output_path = states_dir / file_name
    existing_entries = load_manifest(manifest_path)
    previous_entry = next(
        (entry for entry in existing_entries if entry["stateCode"] == state_code),
        None,
    )

    if previous_entry is not None and previous_entry["file"] != file_name:
        previous_path = states_dir / previous_entry["file"]
        if previous_path.exists():
            previous_path.unlink()

    metadata = {
        "iso3Code": "",
        "status": DEFAULT_STATE_STATUS,
        "continent": DEFAULT_CONTINENT,
        "region": DEFAULT_REGION,
        "frenchShortName": state_name,
    }

    output_path.write_text(
        render_boundary_source(boundary_code, state_name, metadata, polygons, generated_function_name),
        encoding="utf-8",
    )

    manifest = [entry for entry in existing_entries if entry["stateCode"] != state_code]
    manifest.append(
        {
            "stateCode": state_code,
            "boundaryCode": boundary_code,
            "stateName": state_name,
            "stateFips": state_record["statefp"],
            "countryCode": DEFAULT_COUNTRY_CODE,
            "countryName": DEFAULT_COUNTRY_NAME,
            "countyCount": len(counties),
            "file": file_name,
            "function": generated_function_name,
        }
    )
    manifest.sort(key=lambda entry: entry["stateCode"])
    write_manifest(manifest_path, manifest)
    registry_header_path.write_text(render_registry_header(), encoding="utf-8")
    registry_path.write_text(render_registry(manifest), encoding="utf-8")

    return {
        "stateCode": state_code,
        "stateName": state_name,
        "countyCount": str(len(counties)),
        "file": file_name,
    }


def select_states(args: argparse.Namespace, available_states: list[dict[str, object]]) -> list[dict[str, object]]:
    state_index = {str(entry["stusab"]).upper(): entry for entry in available_states}

    if args.state_code:
        state_code = args.state_code.strip().upper()
        if state_code not in state_index:
            raise ValueError(f"state code {state_code} not found in dataset")
        return [state_index[state_code]]

    if not args.all:
        raise ValueError("either --state-code or --all must be supplied")

    allowed_codes = set(VALID_STATE_CODES)
    if args.include_dc:
        allowed_codes.add("DC")
    if args.include_territories:
        allowed_codes.update({"AS", "GU", "MP", "PR", "VI"})

    return [entry for entry in available_states if str(entry["stusab"]).upper() in allowed_codes]


def main() -> None:
    args = parse_args()
    project_root = args.project_root.resolve()
    available_states = fetch_available_states(args.dataset_url)
    targets = select_states(args, available_states)

    imported = []
    for state_record in targets:
        state_code = str(state_record["stusab"]).upper()
        counties = fetch_counties_for_state(args.dataset_url, state_code)
        imported.append(import_state(project_root, state_record, counties))
        print(f"Imported {state_record['state_name']} ({state_code}) from {len(counties)} counties")

    print(
        json.dumps(
            {
                "importedCount": len(imported),
                "states": imported,
            },
            indent=2,
        )
    )


if __name__ == "__main__":
    main()
