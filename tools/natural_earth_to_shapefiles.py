#!/usr/bin/env python3
"""Build packaged GrADS land and ocean mask shapefiles from Natural Earth.

This is a maintainer tool.  GDAL is required to generate the files, but it is
not required by GrADS at run time; GrADS reads the generated files through its
existing Shapelib interface.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import os
from pathlib import Path
import sys
import zipfile

from osgeo import gdal, ogr, osr


SCALES = ("110m", "50m", "10m")
MASKS = ("land", "ocean")
EXPECTED_SOURCE_SHA256 = (
    "b9e2e7b3d5c2c59593f6eafc19be2ba87fb04788a90c57ccd6cf32a587b6ecc3"
)
DBF_DATE = bytes((122, 5, 8))  # Natural Earth 5.1.2 release date: 2022-05-08


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def resolve_source(source: Path) -> tuple[str, str, str]:
    """Return the GDAL data source, Natural Earth version, and SHA-256."""
    source = source.resolve()
    if not source.is_file():
        raise ValueError(f"source does not exist or is not a file: {source}")
    source_hash = sha256_file(source)
    if source.suffix.lower() != ".zip":
        version_file = source.parent / "VERSION"
        version = (
            version_file.read_text(encoding="ascii").strip()
            if version_file.is_file()
            else "unknown"
        )
        return source.as_posix(), version, source_hash

    with zipfile.ZipFile(source) as archive:
        members = sorted(
            name for name in archive.namelist() if name.lower().endswith(".gpkg")
        )
        if len(members) != 1:
            raise ValueError(
                f"{source} must contain exactly one GeoPackage; found {len(members)}"
            )
        try:
            version = archive.read("VERSION").decode("ascii").strip()
        except KeyError:
            version = "unknown"
    return f"/vsizip/{source.as_posix()}/{members[0]}", version, source_hash


def polygon_parts(geometry: ogr.Geometry):
    """Yield polygon members, retaining each polygon's interior rings."""
    if geometry is None or geometry.IsEmpty():
        return
    geometry_type = ogr.GT_Flatten(geometry.GetGeometryType())
    if geometry_type == ogr.wkbPolygon:
        yield geometry
        return
    if geometry_type in (ogr.wkbMultiPolygon, ogr.wkbGeometryCollection):
        for index in range(geometry.GetGeometryCount()):
            yield from polygon_parts(geometry.GetGeometryRef(index))


def remove_shapefile(path: Path) -> None:
    for suffix in (".shp", ".shx", ".dbf", ".prj", ".cpg", ".qpj"):
        candidate = path.with_suffix(suffix)
        if candidate.exists():
            candidate.unlink()


def normalize_dbf_date(path: Path) -> None:
    """Remove the current date from the DBF header for reproducible output."""
    with path.open("r+b") as stream:
        header = stream.read(4)
        if len(header) != 4:
            raise ValueError(f"invalid DBF header: {path}")
        stream.seek(1)
        stream.write(DBF_DATE)


def geometry_stats(geometry: ogr.Geometry) -> tuple[int, int]:
    rings = geometry.GetGeometryCount()
    vertices = sum(
        geometry.GetGeometryRef(index).GetPointCount() for index in range(rings)
    )
    return rings, vertices


def build_layer(
    source_dataset: gdal.Dataset, output_directory: Path, scale: str, mask: str
) -> dict:
    source_name = f"ne_{scale}_{mask}"
    source_layer = source_dataset.GetLayerByName(source_name)
    if source_layer is None:
        raise ValueError(f"required layer is missing: {source_name}")

    output_root = output_directory / f"ne{scale}_{mask}"
    remove_shapefile(output_root)
    driver = ogr.GetDriverByName("ESRI Shapefile")
    output_dataset = driver.CreateDataSource(output_root.with_suffix(".shp").as_posix())
    if output_dataset is None:
        raise RuntimeError(f"could not create {output_root}.shp")

    spatial_reference = osr.SpatialReference()
    spatial_reference.ImportFromEPSG(4326)
    spatial_reference.SetAxisMappingStrategy(osr.OAMS_TRADITIONAL_GIS_ORDER)
    output_layer = output_dataset.CreateLayer(
        output_root.name, spatial_reference, ogr.wkbPolygon
    )
    output_layer.CreateField(ogr.FieldDefn("source_id", ogr.OFTInteger64))
    output_definition = output_layer.GetLayerDefn()

    source_features = 0
    output_features = 0
    repaired_features = 0
    rings = 0
    vertices = 0
    source_layer.ResetReading()
    for source_feature in source_layer:
        source_features += 1
        geometry = source_feature.GetGeometryRef()
        if geometry is None or geometry.IsEmpty():
            continue
        geometry = geometry.Clone()
        geometry.FlattenTo2D()
        if not geometry.IsValid():
            geometry = geometry.MakeValid()
            repaired_features += 1
        for polygon in polygon_parts(geometry):
            polygon = polygon.Clone()
            polygon.CloseRings()
            if polygon.IsEmpty() or not polygon.IsValid():
                raise ValueError(
                    f"invalid polygon after repair in {source_name}, feature "
                    f"{source_feature.GetFID()}"
                )
            output_feature = ogr.Feature(output_definition)
            output_feature.SetField("source_id", source_feature.GetFID())
            output_feature.SetGeometry(polygon)
            if output_layer.CreateFeature(output_feature) != ogr.OGRERR_NONE:
                raise RuntimeError(f"failed writing polygon to {output_root}.shp")
            ring_count, vertex_count = geometry_stats(polygon)
            rings += ring_count
            vertices += vertex_count
            output_features += 1
            output_feature = None
    output_dataset = None

    normalize_dbf_date(output_root.with_suffix(".dbf"))
    component_hashes = {}
    component_sizes = {}
    for suffix in (".shp", ".shx", ".dbf", ".prj"):
        component = output_root.with_suffix(suffix)
        if not component.is_file():
            raise ValueError(f"missing generated shapefile component: {component}")
        component_hashes[suffix[1:]] = sha256_file(component)
        component_sizes[suffix[1:]] = component.stat().st_size

    return {
        "source_layer": source_name,
        "source_features": source_features,
        "features": output_features,
        "repaired_source_features": repaired_features,
        "rings": rings,
        "vertices": vertices,
        "bytes": component_sizes,
        "sha256": component_hashes,
    }


def parse_args(argv: list[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("source", type=Path, help="Natural Earth GeoPackage or zip")
    parser.add_argument("output_directory", type=Path, help="GrADS data directory")
    parser.add_argument(
        "--allow-source-mismatch",
        action="store_true",
        help="allow an input archive other than the pinned Natural Earth release",
    )
    return parser.parse_args(argv)


def main(argv: list[str] | None = None) -> int:
    args = parse_args(sys.argv[1:] if argv is None else argv)
    gdal.UseExceptions()
    source_name, version, source_hash = resolve_source(args.source)
    if source_hash != EXPECTED_SOURCE_SHA256 and not args.allow_source_mismatch:
        raise ValueError(
            "Natural Earth archive SHA-256 does not match the pinned source: "
            f"{source_hash}"
        )
    args.output_directory.mkdir(parents=True, exist_ok=True)
    dataset = gdal.OpenEx(source_name, gdal.OF_VECTOR | gdal.OF_READONLY)
    if dataset is None:
        raise RuntimeError(f"GDAL could not open {args.source}")

    manifest = {
        "format": 1,
        "license": "public-domain",
        "natural_earth_version": version,
        "project_url": "https://www.naturalearthdata.com/",
        "source_sha256": source_hash,
        "layers": {},
    }
    for scale in SCALES:
        for mask in MASKS:
            name = f"ne{scale}_{mask}"
            stats = build_layer(dataset, args.output_directory, scale, mask)
            manifest["layers"][name] = stats
            print(
                f"{name}: {stats['features']} polygons, {stats['rings']} rings, "
                f"{stats['vertices']} vertices"
            )
    dataset = None

    manifest_path = args.output_directory / "natural_earth_masks.json"
    temporary_path = manifest_path.with_suffix(".json.tmp")
    temporary_path.write_text(
        json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8"
    )
    os.replace(temporary_path, manifest_path)
    print(f"Wrote {manifest_path}")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (OSError, RuntimeError, ValueError, zipfile.BadZipFile) as error:
        print(f"error: {error}", file=sys.stderr)
        raise SystemExit(1)
