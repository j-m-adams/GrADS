"""
gradspy - Python interface to GrADS using ctypes + xarray.

Provides the :class:`GrADS` class for interacting with the GrADS (Grid
Analysis and Display System) engine via its shared library.
All grid results are returned as :class:`xarray.DataArray` objects with
named dimension coordinates (``lon``, ``lat``, ``lev``, ``time``,
``ensemble``).  Non-varying dimensions are attached as scalar coordinates
so the full GrADS environment is preserved.

The library to load is resolved in this order:

1. The ``lib`` argument to :class:`GrADS`.
2. The ``$GAGPY`` environment variable.
3. The platform default (``libgradspy.so``, ``libgradspy.dylib``, or
   ``gradspy.dll``) on the system library path.

Typical usage::

    from gradspy import GrADS
    import numpy as np

    ga = GrADS()
    ga.start("-b")
    ga.cmd("open /path/to/myfile.ctl")

    sst = ga.result("sst")              # xr.DataArray(lat, lon)
    sst_c = sst - 273.15               # works directly with xarray
    sst_c.attrs.update(sst.attrs)      # preserve GrADS metadata
    ga.put("sst_c", sst_c)

    ga.cmd("quit")

Copyright (C) 1988-2022 by George Mason University. See file COPYRIGHT for
more information.
"""

import ctypes
import os
import sys
from typing import Optional

import numpy as np
import pandas as pd
import xarray as xr


# ---------------------------------------------------------------------------
# C structure mirroring gradspy.h: struct pygagrid
# ---------------------------------------------------------------------------

class _PyGaGrid(ctypes.Structure):
    """Mirror of ``struct pygagrid`` defined in gradspy.h."""

    _fields_ = [
        ("gastatptr", ctypes.c_void_p),
        ("grid",      ctypes.POINTER(ctypes.c_double)),
        ("isiz",      ctypes.c_int),
        ("jsiz",      ctypes.c_int),
        ("idim",      ctypes.c_int),
        ("jdim",      ctypes.c_int),
        ("xsz",       ctypes.c_int),
        ("ysz",       ctypes.c_int),
        ("zsz",       ctypes.c_int),
        ("tsz",       ctypes.c_int),
        ("esz",       ctypes.c_int),
        # --- 4 bytes of C compiler padding here (double alignment) ---
        ("xstrt",     ctypes.c_double),
        ("ystrt",     ctypes.c_double),
        ("zstrt",     ctypes.c_double),
        ("xincr",     ctypes.c_double),
        ("yincr",     ctypes.c_double),
        ("zincr",     ctypes.c_double),
        ("syr",       ctypes.c_int),
        ("smo",       ctypes.c_int),
        ("sdy",       ctypes.c_int),
        ("shr",       ctypes.c_int),
        ("smn",       ctypes.c_int),
        ("tincr",     ctypes.c_int),
        ("ttyp",      ctypes.c_int),
        ("tcal",      ctypes.c_int),
        ("estrt",     ctypes.c_int),
        # --- 4 bytes of C compiler padding here (pointer alignment) ---
        ("xvals",     ctypes.POINTER(ctypes.c_double)),
        ("yvals",     ctypes.POINTER(ctypes.c_double)),
        ("zvals",     ctypes.POINTER(ctypes.c_double)),
    ]


# ---------------------------------------------------------------------------
# Dimension name constants
# ---------------------------------------------------------------------------

# Maps GrADS dimension index (0–4) to the standard DataArray dim name.
_IDX_TO_DIM = {0: 'lon', 1: 'lat', 2: 'lev', 3: 'time', 4: 'ensemble'}
_DIM_TO_IDX = {v: k for k, v in _IDX_TO_DIM.items()}

# Accepted aliases that map to the 5 standard names used in put().
_ALIASES = {
    'longitude': 'lon',  'x': 'lon',
    'latitude':  'lat',  'y': 'lat',
    'level':     'lev',  'z': 'lev',  'pressure': 'lev',
    't':         'time',
    'ens':       'ensemble',           'e': 'ensemble',
}

# Expected dim order in GrADS memory: E slowest … X fastest.
_GrADS_DIM_ORDER = ['ensemble', 'time', 'lev', 'lat', 'lon']


# ---------------------------------------------------------------------------
# GrADS interface class
# ---------------------------------------------------------------------------

class GrADS:
    """
    Python interface to the GrADS engine via ctypes.

    All grid methods return :class:`xarray.DataArray` objects.
    Non-varying GrADS dimensions appear as scalar coordinates so the full
    dimension environment is available for inspection and roundtrip ``put()``.

    Example
    -------
    >>> ga = GrADS()
    >>> ga.start("-b")
    >>> ga.cmd("open myfile.ctl")
    >>> sst = ga.result("sst")          # DataArray(lat, lon)
    >>> ga.put("sst2", sst * 2)
    >>> ga.cmd("quit")
    """

    def __init__(self, lib: Optional[str] = None) -> None:
        """
        Load the GrADS shared library.

        Parameters
        ----------
        lib : str, optional
            Full path to the GradsPy shared library. Falls back to ``$GAGPY``,
            then the platform-specific library name on the system path.
        """
        if sys.platform == "win32":
            default_library = "gradspy.dll"
        elif sys.platform == "darwin":
            default_library = "libgradspy.dylib"
        else:
            default_library = "libgradspy.so"

        lib_path = lib or os.environ.get("GAGPY", default_library)
        if sys.platform == "win32":
            self._lib = ctypes.CDLL(lib_path)
        else:
            self._lib = ctypes.CDLL(lib_path, mode=ctypes.RTLD_GLOBAL)

        self._setup_signatures()
        self._started = False

    # -----------------------------------------------------------------------
    # Public API
    # -----------------------------------------------------------------------

    def start(self, *args: str) -> int:
        """
        Initialise the GrADS engine.

        Parameters
        ----------
        *args : str
            Command-line arguments for GrADS, e.g. ``'-b'`` for batch mode.

        Returns
        -------
        int
            0 on success.
        """
        if self._started:
            raise RuntimeError("GrADS has already been started")
        argv   = ["gradspy"] + list(args)
        c_argv = (ctypes.c_char_p * len(argv))(*[a.encode() for a in argv])
        rc     = self._lib.gamain(len(argv), c_argv)
        if rc == 0:
            self._started = True
        return rc

    def cmd(self, command: str) -> str:
        """
        Execute a GrADS command and return any output text.

        Parameters
        ----------
        command : str
            A GrADS command string, e.g. ``'open myfile.ctl'``.

        Returns
        -------
        str
            Text output from GrADS, or an empty string.
        """
        self._require_started()
        rc  = ctypes.c_int()
        ptr = self._lib.gagsdo(command.encode(), ctypes.byref(rc))
        if rc.value < 0:
            raise SystemExit("GrADS requested exit")
        if not ptr:
            return ""
        text = ctypes.cast(ptr, ctypes.c_char_p).value.decode()
        self._lib.gapystrfree(ptr)
        return text

    def result(self, expr: str) -> xr.DataArray:
        """
        Evaluate a GrADS expression and return the result as a DataArray.

        The current GrADS dimension environment determines which dimensions
        vary.  For ≤2 varying dimensions the fast ``gadoexpr`` path is used;
        for higher-rank results (e.g. lat × lon × time) the method
        automatically falls back to a ``define`` / ``gadoget`` / ``undefine``
        round-trip so the full N-D array is returned.

        Parameters
        ----------
        expr : str
            A GrADS expression, e.g. ``'ts'``, ``'ua - ub'``,
            ``'ave(ts, t=1, t=12)'``.

        Returns
        -------
        xarray.DataArray

        Raises
        ------
        ValueError
            If GrADS reports an error evaluating the expression.
        """
        self._require_started()
        pygr = _PyGaGrid()
        rank = self._lib.gadoexpr(expr.encode(), ctypes.byref(pygr))
        if rank >= 0:
            da = self._pygr_to_da_2d(rank, pygr, expr)
            self._lib.gapyfre(ctypes.byref(pygr))
            return da
        self._lib.gapyfre(ctypes.byref(pygr))

        # gadoexpr failed — likely > 2 varying dims.  Fall back to define/get.
        _TMP = "pytmp"
        self.cmd(f"define {_TMP} = {expr}")
        try:
            pygr2 = _PyGaGrid()
            rank2 = self._lib.gadoget(_TMP.encode(), ctypes.byref(pygr2))
            if rank2 < 0:
                raise ValueError(f"GrADS expression error: '{expr}'")
            da = self._pygr_to_da_nd(rank2, pygr2, expr)
            self._lib.gapyfre(ctypes.byref(pygr2))
        except ValueError:
            raise
        finally:
            self.cmd(f"undefine {_TMP}")
        return da

    def get(self, name: str) -> xr.DataArray:
        """
        Retrieve a GrADS defined variable as a DataArray.

        Unlike :meth:`result`, this reads the full multi-dimensional
        defined variable without filtering through the dimension environment.

        Parameters
        ----------
        name : str
            Name of the defined variable, e.g. ``'myvar'``.

        Returns
        -------
        xarray.DataArray

        Raises
        ------
        KeyError
            If the named variable does not exist in GrADS.
        """
        self._require_started()
        pygr = _PyGaGrid()
        rank = self._lib.gadoget(name.encode(), ctypes.byref(pygr))
        if rank < 0:
            raise KeyError(f"GrADS defined variable not found: '{name}'")
        da = self._pygr_to_da_nd(rank, pygr, name)
        self._lib.gapyfre(ctypes.byref(pygr))
        return da

    def put(self, name: str, da: xr.DataArray) -> None:
        """
        Create or replace a GrADS defined variable from a DataArray.

        Dimension names must be one of ``lon``, ``lat``, ``lev``, ``time``,
        ``ensemble`` (or the aliases listed in ``gradspy._ALIASES``).
        Scalar coordinates produced by :meth:`result` or :meth:`get` are
        automatically used for the fixed GrADS dimensions.

        The GrADS time metadata (``grads_time_type``, ``grads_calendar``)
        stored in ``da.attrs`` by :meth:`result`/:meth:`get` is used for
        the roundtrip; when absent it defaults to monthly increments on
        the Gregorian calendar.

        Parameters
        ----------
        name : str
            Variable name: lowercase alphanumeric, starts with a letter,
            max 16 characters.
        da : xarray.DataArray
            Data to store.

        Raises
        ------
        RuntimeError
            If GrADS reports an error creating the variable.
        """
        self._require_started()

        # Normalise dim names via aliases
        rename = {d: _ALIASES.get(d, d) for d in da.dims if d in _ALIASES}
        if rename:
            da = da.rename(rename)

        # Reorder dims to match GrADS memory layout (E > T > Z > Y > X)
        present = [d for d in _GrADS_DIM_ORDER if d in da.dims]
        if list(da.dims) != present:
            da = da.transpose(*present)

        pygr           = _PyGaGrid()
        pygr.gastatptr = None

        # -- populate each GrADS axis --
        c_xv = c_yv = c_zv = None  # keep alive until gasetup returns

        for gidx, dim_name, set_size in (
            (0, 'lon',      lambda n: setattr(pygr, 'xsz', n)),
            (1, 'lat',      lambda n: setattr(pygr, 'ysz', n)),
            (2, 'lev',      lambda n: setattr(pygr, 'zsz', n)),
            (3, 'time',     lambda n: setattr(pygr, 'tsz', n)),
            (4, 'ensemble', lambda n: setattr(pygr, 'esz', n)),
        ):
            if dim_name in da.dims:
                n    = da.sizes[dim_name]
                vals = da.coords[dim_name].values if dim_name in da.coords else None
                set_size(n)
            elif dim_name in da.coords:          # scalar (fixed) coordinate
                n    = 1
                vals = np.atleast_1d(da.coords[dim_name].values)
                set_size(1)
            else:
                set_size(1)
                vals = None
                n    = 1

            if gidx == 0:   # X
                strt, incr, nlin = self._parse_axis(vals, n)
                pygr.xstrt = strt;  pygr.xincr = incr
                if nlin is not None:
                    c_xv = (ctypes.c_double * n)(*nlin);  pygr.xvals = c_xv
            elif gidx == 1: # Y
                strt, incr, nlin = self._parse_axis(vals, n)
                pygr.ystrt = strt;  pygr.yincr = incr
                if nlin is not None:
                    c_yv = (ctypes.c_double * n)(*nlin);  pygr.yvals = c_yv
            elif gidx == 2: # Z
                strt, incr, nlin = self._parse_axis(vals, n)
                pygr.zstrt = strt;  pygr.zincr = incr
                if nlin is not None:
                    c_zv = (ctypes.c_double * n)(*nlin);  pygr.zvals = c_zv
            elif gidx == 3: # T
                self._fill_time(pygr, vals, da.attrs)
            elif gidx == 4: # E
                pygr.estrt = int(vals[0]) if vals is not None and len(vals) else 1

        flat  = np.asarray(da.values, dtype=np.float64).ravel()
        c_buf = (ctypes.c_double * len(flat))(*flat)
        pygr.grid = c_buf

        rc = self._lib.gasetup(name.encode(), ctypes.byref(pygr))
        if rc < 0:
            raise RuntimeError(f"GrADS gasetup failed for '{name}' (rc={rc})")

    # -----------------------------------------------------------------------
    # Private: ctypes setup
    # -----------------------------------------------------------------------

    def _setup_signatures(self) -> None:
        lib = self._lib

        lib.gamain.restype   = ctypes.c_int
        lib.gamain.argtypes  = [ctypes.c_int, ctypes.POINTER(ctypes.c_char_p)]

        lib.gagsdo.restype   = ctypes.c_void_p   # malloc'd; freed after copy
        lib.gagsdo.argtypes  = [ctypes.c_char_p, ctypes.POINTER(ctypes.c_int)]

        lib.gapystrfree.restype  = None
        lib.gapystrfree.argtypes = [ctypes.c_void_p]

        lib.gadoexpr.restype  = ctypes.c_int
        lib.gadoexpr.argtypes = [ctypes.c_char_p, ctypes.POINTER(_PyGaGrid)]

        lib.gadoget.restype   = ctypes.c_int
        lib.gadoget.argtypes  = [ctypes.c_char_p, ctypes.POINTER(_PyGaGrid)]

        lib.gasetup.restype   = ctypes.c_int
        lib.gasetup.argtypes  = [ctypes.c_char_p, ctypes.POINTER(_PyGaGrid)]

        lib.gapyfre.restype   = None
        lib.gapyfre.argtypes  = [ctypes.POINTER(_PyGaGrid)]

    def _require_started(self) -> None:
        if not self._started:
            raise RuntimeError("Call start() before using GrADS methods")

    # -----------------------------------------------------------------------
    # Private: DataArray construction
    # -----------------------------------------------------------------------

    def _pygr_to_da_2d(self, rank: int, pygr: _PyGaGrid, expr: str = "") -> xr.DataArray:
        """Build a DataArray from a 2-D pygr struct (gadoexpr result)."""
        n      = pygr.isiz * pygr.jsiz
        values = np.array(pygr.grid[:n], dtype=np.float64).reshape(
            pygr.jsiz, pygr.isiz
        )
        all_c  = self._all_coords(pygr, expr)

        if rank == 0:
            dims, dim_coords = [], {}
            values = float(values[0, 0])
        elif rank == 1:
            d = _IDX_TO_DIM.get(pygr.idim, f'dim{pygr.idim}')
            dims, dim_coords = [d], {d: all_c[pygr.idim]}
            values = values[0]
        else:
            jd = _IDX_TO_DIM.get(pygr.jdim, f'dim{pygr.jdim}')
            id_ = _IDX_TO_DIM.get(pygr.idim, f'dim{pygr.idim}')
            dims      = [jd, id_]
            dim_coords = {jd: all_c[pygr.jdim], id_: all_c[pygr.idim]}

        # Fixed dimensions → scalar coordinates
        scalar_coords = {
            _IDX_TO_DIM[i]: all_c[i][0]
            for i in range(5)
            if _IDX_TO_DIM[i] not in dims and len(all_c[i]) > 0
        }

        return xr.DataArray(
            values,
            dims=dims,
            coords={**dim_coords, **scalar_coords},
            attrs=self._grads_attrs(pygr),
        )

    def _pygr_to_da_nd(self, rank: int, pygr: _PyGaGrid, expr: str = "") -> xr.DataArray:
        """Build a DataArray from an N-D pygr struct (gadoget result)."""
        gsz     = pygr.xsz * pygr.ysz * pygr.zsz * pygr.tsz * pygr.esz
        all_c   = self._all_coords(pygr, expr)

        # Build dims in (E, T, Z, Y, X) order, skipping size-1 dimensions
        sizes   = [pygr.esz, pygr.tsz, pygr.zsz, pygr.ysz, pygr.xsz]
        indices = [4, 3, 2, 1, 0]
        varying = [(i, _IDX_TO_DIM[i]) for i, sz in zip(indices, sizes) if sz > 1]

        shape  = tuple(sz for sz in sizes if sz > 1) or (1,)
        values = np.array(pygr.grid[:gsz], dtype=np.float64).reshape(shape)

        dims       = [name for _, name in varying]
        dim_coords = {name: all_c[i] for i, name in varying}
        scalar_coords = {
            _IDX_TO_DIM[i]: all_c[i][0]
            for i, sz in zip(indices, sizes)
            if sz == 1 and len(all_c[i]) > 0
        }

        return xr.DataArray(
            values,
            dims=dims,
            coords={**dim_coords, **scalar_coords},
            attrs=self._grads_attrs(pygr),
        )

    # -----------------------------------------------------------------------
    # Private: coordinate helpers
    # -----------------------------------------------------------------------

    def _all_coords(self, pygr: _PyGaGrid, expr: str = "") -> dict:
        """Return {dim_index: 1-D coordinate array} for all 5 GrADS dims."""
        # X/Y: xvals/yvals hold world coords when varying (sz>1);
        #      when fixed (sz==1), xvals[0]/yvals[0] = NaN but xstrt/ystrt = world coord.
        x = (np.array([pygr.xstrt]) if pygr.xsz == 1
             else np.array(pygr.xvals[:pygr.xsz], dtype=np.float64))
        y = (np.array([pygr.ystrt]) if pygr.ysz == 1
             else np.array(pygr.yvals[:pygr.ysz], dtype=np.float64))
        return {
            0: x,
            1: y,
            2: self._z_coord(pygr, expr),
            3: self._time_coord(pygr),
            4: np.arange(pygr.estrt, pygr.estrt + pygr.esz, dtype=int),
        }

    def _z_coord(self, pygr: _PyGaGrid, expr: str = "") -> np.ndarray:
        """Build the Z (level) coordinate array from pygr struct.

        GrADS quirk: for a fixed single level (zsz==1), zvals[0]=NaN and
        zstrt=1 (hardcoded, not the world coordinate).  For varying levels
        with non-linear spacing (zincr<0), zvals contains world coords.
        For varying levels with linear spacing, reconstruct from zstrt+zincr.
        """
        if pygr.zsz == 1:
            lev = self._query_fixed_lev(expr)
            return np.array([lev if lev is not None else np.nan])
        if pygr.zincr < 0:  # non-linear: zvals contains the world coordinates
            return np.array(pygr.zvals[:pygr.zsz], dtype=np.float64)
        # linear varying: zvals not filled by GrADS; reconstruct from strt+incr
        return pygr.zstrt + np.arange(pygr.zsz) * pygr.zincr

    def _query_fixed_lev(self, expr: str = "") -> Optional[float]:
        """Return the fixed Z world coordinate, or None for surface variables.

        For simple variable names, ``q file`` is checked to see if the
        variable has 0 levels (surface-only); if so, NaN is appropriate.
        For complex expressions the current level from ``q dims`` is used.
        """
        import re
        name = expr.strip()
        if re.match(r'^[a-z][a-z0-9]{0,15}$', name):
            for line in self.cmd("q file").splitlines():
                parts = line.split()
                if len(parts) >= 3 and parts[0] == name:
                    try:
                        if int(parts[1]) == 0:
                            return None   # surface variable — no level
                    except ValueError:
                        pass
                    break  # found the variable; fall through to q dims
        for line in self.cmd("q dims").splitlines():
            if line.startswith("Z is fixed"):
                try:
                    return float(line.split("Lev =")[1].split()[0])
                except (IndexError, ValueError):
                    pass
        return None

    def _time_coord(self, pygr: _PyGaGrid) -> np.ndarray:
        """Build a time coordinate array from GrADS time metadata."""
        try:
            mo = max(pygr.smo, 1);  dy = max(pygr.sdy, 1)
            t0 = pd.Timestamp(pygr.syr, mo, dy, pygr.shr, pygr.smn)
        except Exception:
            return np.arange(pygr.tsz, dtype=float)

        if pygr.ttyp == 1:   # minute-based increment
            times = pd.date_range(t0, periods=pygr.tsz,
                                  freq=pd.Timedelta(minutes=pygr.tincr))
        else:                # month-based increment
            def _add_months(ts, n):
                total = (ts.month - 1) + n
                return pd.Timestamp(ts.year + total // 12,
                                    total % 12 + 1,
                                    ts.day, ts.hour, ts.minute)
            times = pd.DatetimeIndex(
                [_add_months(t0, i * pygr.tincr) for i in range(pygr.tsz)]
            )

        return times.values   # numpy datetime64[ns]

    def _grads_attrs(self, pygr: _PyGaGrid) -> dict:
        """Minimal GrADS metadata stored in attrs for roundtrip put()."""
        return {
            'grads_time_type': pygr.ttyp,   # 0 = months, 1 = minutes
            'grads_calendar':  pygr.tcal,   # 0 = Gregorian, 1 = 365-day
        }

    # -----------------------------------------------------------------------
    # Private: put() helpers
    # -----------------------------------------------------------------------

    @staticmethod
    def _parse_axis(vals, n: int):
        """
        Determine if a coordinate axis is linear or non-linear.

        Returns (start, incr, nonlinear_vals_or_None).
        Linear  → incr > 0,  nonlinear_vals = None.
        Non-linear → incr = -1, nonlinear_vals = float array.
        """
        if vals is None or n <= 1:
            start = float(vals[0]) if vals is not None and len(vals) else 0.0
            return start, 1.0, None

        fvals = vals.astype(float)
        start = fvals[0]
        expected_incr = (fvals[-1] - fvals[0]) / (n - 1)

        if np.allclose(np.diff(fvals), expected_incr, rtol=1e-4, atol=1e-6):
            return start, expected_incr, None    # linear
        return start, -1.0, fvals               # non-linear

    @staticmethod
    def _fill_time(pygr: _PyGaGrid, time_vals, attrs: dict) -> None:
        """Populate pygr time fields from a datetime64 coordinate array."""
        pygr.ttyp  = int(attrs.get('grads_time_type', 0))
        pygr.tcal  = int(attrs.get('grads_calendar',  0))

        if time_vals is None or len(time_vals) == 0:
            pygr.syr = pygr.smo = pygr.sdy = pygr.shr = pygr.smn = 0
            pygr.tincr = 1
            return

        t0 = pd.Timestamp(time_vals[0])
        pygr.syr = t0.year;  pygr.smo = t0.month;  pygr.sdy = t0.day
        pygr.shr = t0.hour;  pygr.smn = t0.minute

        if len(time_vals) > 1:
            t1 = pd.Timestamp(time_vals[1])
            if pygr.ttyp == 1:  # minutes
                pygr.tincr = int(
                    (t1 - t0).total_seconds() / 60
                )
            else:               # months
                pygr.tincr = (t1.year - t0.year) * 12 + (t1.month - t0.month)
        else:
            pygr.tincr = 1
