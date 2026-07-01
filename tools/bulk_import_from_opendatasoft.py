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
        description=(
            "Download and import country records from the Opendatasoft world administrative "
            "boundaries dataset."
        )
    )
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
    parser.add_argument(
        "--where",
        default='status="Member State"',
        help='Dataset filter. Defaults to status="Member State".',
    )
    parser.add_argument(
        "--replace-existing",
        action="store_true",
        help="Re-import countries already present in the manifest.",
    )
    parser.add_argument(
        "--limit",
        type=int,
        default=100,
        help="Page size for the remote records API.",
    )
    return parser.parse_args()


def fetch_records(dataset_url: str, where: str, limit: int) -> list[dict]:
    offset = 0
    records: list[dict] = []

    while True:
        query = urllib.parse.urlencode(
            {
                "where": where,
                "limit": limit,
                "offset": offset,
                "order_by": "iso_3166_1_alpha_2_codes asc",
            }
        )
        url = f"{dataset_url}?{query}"

        with urllib.request.urlopen(url) as response:
            payload = json.load(response)

        page = payload.get("results", [])
        if not page:
            break

        records.extend(page)
        if len(page) < limit:
            break

        offset += len(page)

    return records


def load_existing_codes(project_root: Path) -> set[str]:
    manifest_path = project_root / "data" / "countries" / "manifest.json"
    if not manifest_path.exists():
        return set()

    manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    return {entry["isoCountryCode"] for entry in manifest}


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
            record["name"],
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
    project_root = args.project_root.resolve()
    existing_codes = load_existing_codes(project_root)
    records = fetch_records(args.dataset_url, args.where, args.limit)

    imported = 0
    skipped = 0

    for record in records:
        iso_country_code = (record.get("iso_3166_1_alpha_2_codes") or "").strip().upper()
        if not iso_country_code:
            continue

        if not args.replace_existing and iso_country_code in existing_codes:
            skipped += 1
            continue

        save_source_record(project_root, iso_country_code, record)
        run_importer(project_root, iso_country_code, record)
        imported += 1

    print(
        json.dumps(
            {
                "fetched": len(records),
                "imported": imported,
                "skipped": skipped,
                "replaceExisting": args.replace_existing,
                "where": args.where,
            },
            indent=2,
        )
    )


if __name__ == "__main__":
    main()
