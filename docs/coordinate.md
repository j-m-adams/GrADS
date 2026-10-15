---
title: Coordinate Systems
---

# Coordinate Systems

A major strength of GrADS is its support for a universal, external-to-the-data
**world coordinate**. This lets you slice and compare data sets even when their
underlying binary layouts differ.

## Coordinate systems

GrADS uses three coordinate systems:

| System | Purpose |
| --- | --- |
| Grid (index) coordinates | Identify data by array indices. |
| World coordinates | Identify data by physical quantities such as longitude, latitude, level, and time. |
| Plot coordinates | Identify x/y locations in the graphics window. |

The discussion below focuses on grid and world coordinates, which are the most
important for data selection and analysis. Plot coordinates are described in
the display and graphics documentation.

## World coordinates

The four-dimensional world coordinate is defined inside GrADS as
`lon`, `lat`, `lev`, and `time`:

| GrADS coordinate | Typical physical meaning | Convention |
| --- | --- | --- |
| `lon` | Longitude | Degrees east |
| `lat` | Latitude | Degrees north |
| `lev` | Vertical level | Pressure, sigma, or theta levels |
| `time` | Valid time | Minutes, hours, days, or months on the Gregorian calendar |

For example, the physical point 14.3°N, 145°E, 500 mb at 12:00 UTC on 7 March
1996 is represented in GrADS as:

```text
lon;lat;lev;time = 14.3;145.0;500;12:00Z7Mar1996
```

## Grid coordinates

The four-dimensional grid coordinate is defined inside GrADS as `x`, `y`,
`z`, and `t`. A Fortran program might write a four-dimensional array in
the following order:

```fortran
parameter (nx=144,ny=73,nz=17,nt=2)
dimension u(nx,ny,nz,nt)
integer x,y,z,t

do t=1,nt
  do z=1,nz
    write(10) ((u(x,y),x=1,nx),y=1,ny)
  end do
end do
```

GrADS can reference this data in world coordinates regardless of the storage
order. This is a key difference from data-access interfaces that expose only
the file's native dimensions.

## Mapping grid and world coordinates

GrADS maintains one active mapping between grid and world coordinates at a
time. The mapping is based on the default file, which is normally the first
file opened. It is defined by the coordinate cards in the descriptor file and,
optionally, the `OPTIONS` card.

For example:

```text
DSET ^bm.dat
TITLE the best model ver data
OPTIONS yrev zrev
XDEF 72 linear 0.0 5.0
YDEF 46 linear -90 4.0
ZDEF 3 levels 850 500 200
TDEF 1 linear 00z1jan1996 12hr
VARS 1
z 3 0 geopotential height
ENDVARS
```

GrADS always defines the world coordinate internally in this order:

```text
lon, lat, lev, time
```

The data do not have to be stored in that order. In the example, `OPTIONS
yrev zrev` tells GrADS that the data orientation differs from the standard
world-coordinate orientation:

| Dimension | Grid-coordinate behavior in the example |
| --- | --- |
| Longitude | Increases from west to east. |
| Latitude | Decreases as `y` increases: `y=1 -> lat=90`, `y=2 -> lat=86`, and `y=45 -> lat=-90`. |
| Level | Pressure decreases with height; the stored order is 850, 500, 200 mb. |
| Time | Must be linear with equal spacing; the example uses 12-hour increments. |

The coordinate cards can appear in any order in the descriptor file. The
internal world-coordinate order remains `lon,lat,lev,time`.

## Displaying data relative to world coordinates

When comparing data sets with different grid-to-world mappings, specify a
zero- to three-dimensional region in grid or world coordinates and display the
variable. For example:

```text
open psl.ecmwf.ctl
open psl.ncep.ctl
set gxout contour
set t 1
set lon 0 180
set lat 0 90
d psl
```

The first opened file, `psl.ecmwf.ctl`, is the default file, so its
grid-to-world mapping is used. `set t 1` selects the first time step (the
equivalent world-coordinate command would be `set time 00z1mar96` if that is
the corresponding valid time).

