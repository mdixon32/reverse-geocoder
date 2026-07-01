#!/usr/bin/env python3

from __future__ import annotations

import argparse
import json
import subprocess
import urllib.parse
import urllib.request
from pathlib import Path


DEFAULT_DATASET_URL = (
    "https://public.opendatasoft.com/api/explore/v2.1/catalog/datasets/"
    "world-administrative-boundaries/records"
)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Download a country record from the Opendatasoft world administrative boundaries dataset and import it into the SDK."
    )
    parser.add_argument("--iso-country-code", "--iso", dest="iso_country_code", required=True)
    parser.add_argument(
        "--dataset-url",
        default=DEFAULT_DATASET_URL,
        help="Override the Opendatasoft records API URL if needed.",
    )
    parser.add_argument(
        "--project-root",
        type=Path,
        default=Path(__file__).resolve().parents[1],
        help="SDK root path. Defaults to the current SDK directory.",
    )
    return parser.parse_args()


def fetch_record(dataset_url: str, iso_country_code: str) -> dict:
    query = urllib.parse.urlencode(
        {
            "where": f'iso_3166_1_alpha_2_codes="{iso_country_code}"',
            "limit": 1,
        }
    )
    url = f"{dataset_url}?{query}"

    with urllib.request.urlopen(url) as response:
        payload = json.load(response)

    results = payload.get("results", [])
    if not results:
        raise ValueError(f"no dataset record found for ISO code {iso_country_code}")

    return results[0]


def save_source_record(project_root: Path, iso_country_code: str, record: dict) -> Path:
    source_dir = project_root / "data" / "source" / "opendatasoft"
    source_dir.mkdir(parents=True, exist_ok=True)
    output_path = source_dir / f"{iso_country_code}.json"
    output_path.write_text(f"{json.dumps(record, indent=2)}\n", encoding="utf-8")
    return output_path


def save_geoshape(project_root: Path, iso_country_code: str, geoshape: dict) -> Path:
    geoshape_dir = project_root / "data" / "source" / "opendatasoft" / "geoshapes"
    geoshape_dir.mkdir(parents=True, exist_ok=True)
    output_path = geoshape_dir / f"{iso_country_code}.geo.json"
    output_path.write_text(f"{json.dumps(geoshape)}\n", encoding="utf-8")
    return output_path


def run_importer(project_root: Path, iso_country_code: str, record: dict) -> None:
    country_name = record["name"]
    geoshape_path = save_geoshape(project_root, iso_country_code, record["geo_shape"])

    subprocess.run(
        [
            "python3",
            str(project_root / "tools" / "import_country.py"),
            "--project-root",
            str(project_root),
            "--iso-country-code",
            iso_country_code,
            "--country-name",
            country_name,
            "--iso3-code",
            record.get("iso3") or "",
            "--status",
            record.get("status") or "",
            "--continent",
            record.get("continent") or "",
            "--region",
            record.get("region") or "",
            "--french-short-name",
            record.get("french_short") or "",
            "--geoshape-file",
            str(geoshape_path),
        ],
        check=True,
    )


def main() -> None:
    args = parse_args()
    iso_country_code = args.iso_country_code.strip().upper()
    project_root = args.project_root.resolve()

    record = fetch_record(args.dataset_url, iso_country_code)
    source_path = save_source_record(project_root, iso_country_code, record)
    run_importer(project_root, iso_country_code, record)

    print(f"Saved raw Opendatasoft record to {source_path}")
    print(
        json.dumps(
            {
                "iso2": record.get("iso_3166_1_alpha_2_codes"),
                "iso3": record.get("iso3"),
                "name": record.get("name"),
                "status": record.get("status"),
                "continent": record.get("continent"),
                "region": record.get("region"),
                "french_short": record.get("french_short"),
            },
            indent=2,
        )
    )


if __name__ == "__main__":
    main()
