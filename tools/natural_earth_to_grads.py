#!/usr/bin/env python3
"""Convert Natural Earth vector layers to indexed GrADS map datasets.

Requires GDAL's Python bindings (``from osgeo import gdal, ogr``).
"""

from __future__ import annotations

import argparse
import hashlib
import math
import os
from pathlib import Path
import sys
import zipfile

from osgeo import gdal, ogr


SCALES = ("110m", "50m", "10m")
LAYER_SUFFIXES = {
    0: "coastline",
    1: "admin_0_boundary_lines_land",
    2: "admin_1_states_provinces_lines",
}
LON_EDGES = (0.0, 90.0, 180.0, 270.0, 360.0)
LAT_EDGES = (-90.0, -45.0, 0.0, 45.0, 90.0)
QUANTUM = 10_000
MAX_POINTS = 255
EPSILON = 1.0e-10


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def resolve_source(source: Path) -> tuple[str, str, str]:
    """Return (GDAL data source, version, source SHA-256)."""
    source = source.resolve()
    if source.is_dir():
        candidates = sorted(source.glob("*.gpkg"))
        if len(candidates) != 1:
            raise ValueError(
                f"{source} must contain exactly one .gpkg file; found {len(candidates)}"
            )
        source = candidates[0]

    if not source.is_file():
        raise ValueError(f"source does not exist or is not a file: {source}")

    source_hash = sha256_file(source)
    if source.suffix.lower() == ".zip":
        with zipfile.ZipFile(source) as archive:
            gpkg_members = sorted(
                name for name in archive.namelist() if name.lower().endswith(".gpkg")
            )
            if len(gpkg_members) != 1:
                raise ValueError(
                    f"{source} must contain exactly one .gpkg file; "
                    f"found {len(gpkg_members)}"
                )
            try:
                version = archive.read("VERSION").decode("ascii").strip()
            except KeyError:
                version = "unknown"
        vsi_source = f"/vsizip/{source.as_posix()}/{gpkg_members[0]}"
        return vsi_source, version, source_hash

    version_file = source.parent / "VERSION"
    version = version_file.read_text(encoding="ascii").strip() if version_file.is_file() else "unknown"
    return source.as_posix(), version, source_hash


def iter_lines(geometry: ogr.Geometry):
    """Yield coordinate lists from lineal OGR geometries."""
    if geometry is None or geometry.IsEmpty():
        return
    flat_type = ogr.GT_Flatten(geometry.GetGeometryType())
    if flat_type in (ogr.wkbLineString, ogr.wkbLinearRing):
        points = [(geometry.GetX(i), geometry.GetY(i)) for i in range(geometry.GetPointCount())]
        if len(points) >= 2:
            yield points
        return
    for index in range(geometry.GetGeometryCount()):
        yield from iter_lines(geometry.GetGeometryRef(index))


def unwrap(points: list[tuple[float, float]]) -> list[tuple[float, float]]:
    """Make consecutive longitude differences no greater than 180 degrees."""
    first_x, first_y = points[0]
    result = [(first_x, first_y)]
    previous_x = first_x
    for source_x, y in points[1:]:
        x = source_x
        while x - previous_x > 180.0:
            x -= 360.0
        while x - previous_x < -180.0:
            x += 360.0
        result.append((x, y))
        previous_x = x
    return result


def clip_segment(
    p0: tuple[float, float],
    p1: tuple[float, float],
    xmin: float,
    xmax: float,
    ymin: float,
    ymax: float,
) -> tuple[tuple[float, float], tuple[float, float]] | None:
    """Clip one segment with the Liang-Barsky algorithm."""
    x0, y0 = p0
    x1, y1 = p1
    dx = x1 - x0
    dy = y1 - y0
    low = 0.0
    high = 1.0
    for p, q in ((-dx, x0 - xmin), (dx, xmax - x0), (-dy, y0 - ymin), (dy, ymax - y0)):
        if abs(p) <= EPSILON:
            if q < -EPSILON:
                return None
            continue
        ratio = q / p
        if p < 0.0:
            if ratio > high:
                return None
            low = max(low, ratio)
        else:
            if ratio < low:
                return None
            high = min(high, ratio)
    if high - low <= EPSILON:
        return None
    return (
        (x0 + low * dx, y0 + low * dy),
        (x0 + high * dx, y0 + high * dy),
    )


def points_equal(a: tuple[float, float], b: tuple[float, float]) -> bool:
    return abs(a[0] - b[0]) <= EPSILON and abs(a[1] - b[1]) <= EPSILON


def clip_polyline(
    points: list[tuple[float, float]],
    xmin: float,
    xmax: float,
    ymin: float,
    ymax: float,
) -> list[list[tuple[float, float]]]:
    paths: list[list[tuple[float, float]]] = []
    current: list[tuple[float, float]] = []
    for p0, p1 in zip(points, points[1:]):
        clipped = clip_segment(p0, p1, xmin, xmax, ymin, ymax)
        if clipped is None:
            if len(current) >= 2:
                paths.append(current)
            current = []
            continue
        start, end = clipped
        if current and points_equal(current[-1], start):
            if not points_equal(current[-1], end):
                current.append(end)
        else:
            if len(current) >= 2:
                paths.append(current)
            current = [start, end]
    if len(current) >= 2:
        paths.append(current)
    return paths


def quantize_point(point: tuple[float, float]) -> tuple[int, int]:
    lon, lat = point
    encoded_lon = math.floor(lon * QUANTUM + 0.5)
    encoded_lat = math.floor((lat + 90.0) * QUANTUM + 0.5)
    encoded_lon = min(3_600_000, max(0, encoded_lon))
    encoded_lat = min(1_800_000, max(0, encoded_lat))
    return encoded_lon, encoded_lat


def quantize_path(points: list[tuple[float, float]]) -> list[tuple[int, int]]:
    result: list[tuple[int, int]] = []
    for point in points:
        encoded = quantize_point(point)
        if not result or result[-1] != encoded:
            result.append(encoded)
    return result


def split_path(points: list[tuple[int, int]]):
    """Split at 255 points and repeat the prior endpoint in the next record."""
    start = 0
    while len(points) - start > MAX_POINTS:
        yield points[start : start + MAX_POINTS]
        start += MAX_POINTS - 1
    if len(points) - start >= 2:
        yield points[start:]


def candidate_range(low: float, high: float, edges: tuple[float, ...]) -> range:
    indices = [
        index
        for index in range(len(edges) - 1)
        if high >= edges[index] - EPSILON and low <= edges[index + 1] + EPSILON
    ]
    if not indices:
        return range(0)
    return range(indices[0], indices[-1] + 1)


def collect_cells(layer: ogr.Layer) -> tuple[dict[tuple[int, int], list[list[tuple[int, int]]]], int]:
    cells = {(ix, iy): [] for iy in range(4) for ix in range(4)}
    source_features = 0
    layer.ResetReading()
    for feature in layer:
        source_features += 1
        geometry = feature.GetGeometryRef()
        for source_points in iter_lines(geometry):
            if any(not (math.isfinite(x) and math.isfinite(y)) for x, y in source_points):
                raise ValueError(f"non-finite coordinate in layer {layer.GetName()}")
            if any(y < -90.0 - EPSILON or y > 90.0 + EPSILON for _, y in source_points):
                raise ValueError(f"latitude outside [-90, 90] in layer {layer.GetName()}")
            points = unwrap(source_points)
            min_x = min(point[0] for point in points)
            max_x = max(point[0] for point in points)
            min_y = min(point[1] for point in points)
            max_y = max(point[1] for point in points)
            first_shift = math.ceil((0.0 - max_x - EPSILON) / 360.0)
            last_shift = math.floor((360.0 - min_x + EPSILON) / 360.0)
            for shift_number in range(first_shift, last_shift + 1):
                shift = shift_number * 360.0
                shifted = [(x + shift, y) for x, y in points]
                shifted_min_x = min_x + shift
                shifted_max_x = max_x + shift
                for iy in candidate_range(min_y, max_y, LAT_EDGES):
                    for ix in candidate_range(shifted_min_x, shifted_max_x, LON_EDGES):
                        clipped_paths = clip_polyline(
                            shifted,
                            LON_EDGES[ix],
                            LON_EDGES[ix + 1],
                            LAT_EDGES[iy],
                            LAT_EDGES[iy + 1],
                        )
                        for clipped in clipped_paths:
                            quantized = quantize_path(clipped)
                            if len(quantized) >= 2:
                                cells[(ix, iy)].append(quantized)
    return cells, source_features


def encoded_coord(value: float, latitude: bool = False) -> bytes:
    if latitude:
        integer = math.floor((value + 90.0) * QUANTUM + 0.5)
        maximum = 1_800_000
    else:
        integer = math.floor(value * QUANTUM + 0.5)
        maximum = 3_600_000
    if not 0 <= integer <= maximum:
        raise ValueError(f"index coordinate out of range: {value}")
    return integer.to_bytes(3, "big")


def append_index(
    output: bytearray,
    map_type: int,
    bbox: tuple[float, float, float, float],
) -> int:
    position = len(output)
    xmin, xmax, ymin, ymax = bbox
    output.extend((2, map_type, map_type))
    output.extend(b"\0\0\0\0")
    output.extend(encoded_coord(xmin))
    output.extend(encoded_coord(xmax))
    output.extend(encoded_coord(ymin, latitude=True))
    output.extend(encoded_coord(ymax, latitude=True))
    return position


def append_line(output: bytearray, map_type: int, points: list[tuple[int, int]]) -> None:
    output.extend((1, map_type, len(points)))
    for lon, lat in points:
        output.extend(lon.to_bytes(3, "big"))
        output.extend(lat.to_bytes(3, "big"))


def build_dataset(dataset: gdal.Dataset, scale: str) -> tuple[bytes, dict]:
    output = bytearray()
    outer_positions: dict[int, int] = {}
    cell_positions: dict[int, list[int]] = {}
    stats = {"features": {}, "records": {map_type: 0 for map_type in LAYER_SUFFIXES}}

    for map_type, suffix in LAYER_SUFFIXES.items():
        result_layer = False
        if scale == "10m" and map_type == 2:
            polygon_layer = "ne_10m_admin_1_states_provinces"
            sql = f"""
                SELECT
                    ST_LineMerge(
                        ST_Intersection(ST_Boundary(a.geom), ST_Boundary(b.geom))
                    ) AS geom
                FROM {polygon_layer} AS a
                JOIN {polygon_layer} AS b
                  ON a.adm0_a3 = b.adm0_a3
                 AND a.fid < b.fid
                WHERE ST_Intersects(a.geom, b.geom)
            """
            layer_name = f"shared boundaries from {polygon_layer}"
            layer = dataset.ExecuteSQL(sql, dialect="SQLITE")
            result_layer = True
        else:
            layer_name = f"ne_{scale}_{suffix}"
            layer = dataset.GetLayerByName(layer_name)
        if layer is None:
            raise ValueError(f"required layer is missing: {layer_name}")
        outer_positions[map_type] = append_index(output, map_type, (0.0, 360.0, -90.0, 90.0))
        cells, feature_count = collect_cells(layer)
        if result_layer:
            dataset.ReleaseResultSet(layer)
        stats["features"][map_type] = feature_count
        cell_positions[map_type] = []
        for iy in range(4):
            for ix in range(4):
                cell_positions[map_type].append(
                    append_index(
                        output,
                        map_type,
                        (LON_EDGES[ix], LON_EDGES[ix + 1], LAT_EDGES[iy], LAT_EDGES[iy + 1]),
                    )
                )
                for path in cells[(ix, iy)]:
                    for chunk in split_path(path):
                        append_line(output, map_type, chunk)
                        stats["records"][map_type] += 1

    for map_type in LAYER_SUFFIXES:
        next_outer = outer_positions.get(map_type + 1, 0)
        output[outer_positions[map_type] + 3 : outer_positions[map_type] + 7] = next_outer.to_bytes(4, "big")
        positions = cell_positions[map_type]
        for index, position in enumerate(positions):
            target = positions[index + 1] if index + 1 < len(positions) else next_outer
            output[position + 3 : position + 7] = target.to_bytes(4, "big")

    stats["outer_positions"] = outer_positions
    stats["cell_positions"] = cell_positions
    return bytes(output), stats


def verify_dataset(data: bytes, expected: dict | None = None) -> dict:
    """Decode and validate every byte, record, index target, and map class."""
    position = 0
    record_starts: set[int] = set()
    line_counts = {map_type: 0 for map_type in LAYER_SUFFIXES}
    index_counts = {map_type: 0 for map_type in LAYER_SUFFIXES}
    records: list[tuple[int, int, int]] = []
    index_targets: list[tuple[int, int]] = []
    while position < len(data):
        start = position
        if position + 3 > len(data):
            raise ValueError(f"truncated record header at byte {position}")
        record_starts.add(start)
        kind, first, second = data[position : position + 3]
        position += 3
        if kind == 1:
            map_type = first
            records.append((start, kind, map_type))
            point_count = second
            if map_type not in LAYER_SUFFIXES:
                raise ValueError(f"invalid map type {map_type} at byte {start}")
            if not 2 <= point_count <= MAX_POINTS:
                raise ValueError(f"invalid point count {point_count} at byte {start}")
            end = position + point_count * 6
            if end > len(data):
                raise ValueError(f"line record at byte {start} crosses EOF")
            for offset in range(position, end, 6):
                lon = int.from_bytes(data[offset : offset + 3], "big")
                lat = int.from_bytes(data[offset + 3 : offset + 6], "big")
                if lon > 3_600_000 or lat > 1_800_000:
                    raise ValueError(f"coordinate out of range at byte {offset}")
            position = end
            line_counts[map_type] += 1
        elif kind == 2:
            records.append((start, kind, first))
            if first not in LAYER_SUFFIXES or second != first:
                raise ValueError(f"invalid index class range {first}..{second} at byte {start}")
            if position + 16 > len(data):
                raise ValueError(f"index record at byte {start} crosses EOF")
            target = int.from_bytes(data[position : position + 4], "big")
            xmin = int.from_bytes(data[position + 4 : position + 7], "big")
            xmax = int.from_bytes(data[position + 7 : position + 10], "big")
            ymin = int.from_bytes(data[position + 10 : position + 13], "big")
            ymax = int.from_bytes(data[position + 13 : position + 16], "big")
            if not (xmin <= xmax <= 3_600_000 and ymin <= ymax <= 1_800_000):
                raise ValueError(f"invalid index bounds at byte {start}")
            index_targets.append((start, target))
            index_counts[first] += 1
            position += 16
        else:
            raise ValueError(f"invalid record kind {kind} at byte {start}")

    if position != len(data):
        raise ValueError(f"decoder stopped at {position}, not EOF {len(data)}")
    if any(count == 0 for count in line_counts.values()):
        raise ValueError(f"one or more map classes are empty: {line_counts}")
    for start, target in index_targets:
        if target != 0 and target not in record_starts:
            raise ValueError(f"index target {target} is not a record boundary")
        if target != 0 and target <= start:
            raise ValueError(f"index at {start} does not point forward: {target}")

    if expected is not None:
        for map_type in LAYER_SUFFIXES:
            outer = expected["outer_positions"][map_type]
            cells = expected["cell_positions"][map_type]
            next_outer = expected["outer_positions"].get(map_type + 1, 0)
            class_end = next_outer or len(data)
            if data[outer : outer + 3] != bytes((2, map_type, map_type)):
                raise ValueError(f"missing outer index for map type {map_type}")
            actual_outer_target = int.from_bytes(data[outer + 3 : outer + 7], "big")
            if actual_outer_target != next_outer:
                raise ValueError(f"invalid outer index target for map type {map_type}")
            if len(cells) != 16:
                raise ValueError(f"map type {map_type} does not have 16 spatial indexes")
            for index, cell in enumerate(cells):
                if data[cell : cell + 3] != bytes((2, map_type, map_type)):
                    raise ValueError(f"invalid spatial index for map type {map_type} at {cell}")
                expected_target = cells[index + 1] if index + 1 < len(cells) else next_outer
                actual_target = int.from_bytes(data[cell + 3 : cell + 7], "big")
                if actual_target != expected_target:
                    raise ValueError(f"invalid spatial index target at {cell}")
            for record_start, _kind, record_type in records:
                if outer <= record_start < class_end and record_type != map_type:
                    raise ValueError(
                        f"record at {record_start} has class {record_type}, expected {map_type}"
                    )
            if line_counts[map_type] != expected["records"][map_type]:
                raise ValueError(f"record count mismatch for map type {map_type}")
        expected_indexes = {map_type: 17 for map_type in LAYER_SUFFIXES}
        if index_counts != expected_indexes:
            raise ValueError(f"unexpected index counts: {index_counts}")

    return {"line_records": line_counts, "index_records": index_counts, "bytes": len(data)}


def parse_args(argv: list[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("source", type=Path, help="Natural Earth GeoPackage zip, .gpkg, or directory")
    parser.add_argument("output_directory", type=Path, help="directory for ne110m, ne50m, and ne10m")
    return parser.parse_args(argv)


def main(argv: list[str] | None = None) -> int:
    args = parse_args(sys.argv[1:] if argv is None else argv)
    gdal.UseExceptions()
    data_source, version, source_hash = resolve_source(args.source)
    args.output_directory.mkdir(parents=True, exist_ok=True)
    dataset = gdal.OpenEx(data_source, gdal.OF_VECTOR | gdal.OF_READONLY)
    if dataset is None:
        raise RuntimeError(f"GDAL could not open {args.source}")

    print(f"Natural Earth version: {version}")
    print(f"Source SHA-256: {source_hash}")
    for scale in SCALES:
        data, expected = build_dataset(dataset, scale)
        verification = verify_dataset(data, expected)
        output_path = args.output_directory / f"ne{scale}"
        temporary_path = output_path.with_name(output_path.name + ".tmp")
        temporary_path.write_bytes(data)
        os.replace(temporary_path, output_path)
        digest = hashlib.sha256(data).hexdigest()
        features = ", ".join(
            f"type {map_type}: {expected['features'][map_type]} features"
            for map_type in LAYER_SUFFIXES
        )
        records = ", ".join(
            f"type {map_type}: {verification['line_records'][map_type]} records"
            for map_type in LAYER_SUFFIXES
        )
        print(f"{output_path}: {len(data)} bytes; SHA-256 {digest}")
        print(f"  {features}")
        print(f"  {records}; 17 indexes per type")
    dataset = None
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (OSError, RuntimeError, ValueError, zipfile.BadZipFile) as error:
        print(f"error: {error}", file=sys.stderr)
        raise SystemExit(1)
