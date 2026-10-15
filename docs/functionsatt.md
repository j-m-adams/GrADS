---
title: GrADS Functions by Category
---

# GrADS Functions by Category

This page groups the intrinsic GrADS functions by their primary purpose. The
functions within each category are listed alphabetically. For a single
alphabetical list, expand the menu entry on the left.

## Mathematical operations

| Function | Description |
| --- | --- |
| [`abs()`](gradfuncabs.md) | Absolute value |
| [`cdiff()`](gradfunccdiff.md) | Centered difference |
| [`exp()`](gradfuncexp.md) | Exponential |
| [`gint()`](gradfuncgint.md) | General integral |
| [`log()`](gradfunclog.md) | Natural logarithm |
| [`log10()`](gradfunclog10.md) | Base-10 logarithm |
| [`pow()`](gradfuncpow.md) | Raises one argument to the power of another |
| [`sqrt()`](gradfuncsqrt.md) | Square root |
| [`vint()`](gradfuncvint.md) | Mass-weighted vertical integral in pressure coordinates |

## Trigonometric functions

| Function | Description |
| --- | --- |
| [`acos()`](gradfuncacos.md) | Inverse cosine |
| [`asin()`](gradfuncasin.md) | Inverse sine |
| [`atan2()`](gradfuncatan2.md) | Inverse tangent |
| [`cos()`](gradfunccos.md) | Cosine |
| [`sin()`](gradfuncsin.md) | Sine |
| [`tan()`](gradfunctan.md) | Tangent |

## Averaging and summing

| Function | Description |
| --- | --- |
| [`aave()`](gradfuncaave.md) | Latitude-weighted areal average |
| [`amean()`](gradfuncamean.md) | Areal average without latitude weighting |
| [`asum()`](gradfuncasum.md) | Grid-weighted areal sum |
| [`asumg()`](gradfuncasumg.md) | Areal sum without grid weighting |
| [`atot()`](gradfuncatot.md) | Grid- and latitude-weighted areal sum |
| [`ave()`](gradfuncave.md) | Average over any dimension |
| [`mean()`](gradfuncmean.md) | Average over any dimension without latitude weighting |
| [`sum()`](gradfuncsum.md) | Grid-weighted sum over any dimension |
| [`sumg()`](gradfuncsumg.md) | Sum over any dimension without grid weighting |
| [`tmave()`](gradfunctmave.md) | Time average with a mask |

## Correlation and regression

| Function | Description |
| --- | --- |
| [`scorr()`](gradfuncscorr.md) | Spatial correlation over an X–Y domain |
| [`sregr()`](gradfuncsregr.md) | Spatial least-squares regression |
| [`tcorr()`](gradfunctcorr.md) | Temporal correlation coefficients |
| [`tregr()`](gradfunctregr.md) | Least-squares regression over time |

## Meteorological calculations

| Function | Description |
| --- | --- |
| [`tvrh2q()`](gradfunctvrh2q.md) | Specific humidity from virtual temperature and relative humidity |
| [`tvrh2t()`](gradfunctvrh2t.md) | Temperature from virtual temperature and relative humidity |

## Vector operations

| Function | Description |
| --- | --- |
| [`hcurl()`](gradfunchcurl.md) | Vertical component of the curl |
| [`hdivg()`](gradfunchdivg.md) | Horizontal divergence |
| [`mag()`](gradfuncmag.md) | Wind speed from the *u* and *v* components |
| [`skip()`](gradfuncskip.md) | Sets alternating values to missing |

## Grid operations

| Function | Description |
| --- | --- |
| [`amax()`](gradfuncamax.md) | Maximum over an X–Y region |
| [`amaxlocx()`](gradfuncamaxlocx.md) | X location of the regional maximum |
| [`amaxlocy()`](gradfuncamaxlocy.md) | Y location of the regional maximum |
| [`amin()`](gradfuncamin.md) | Minimum over an X–Y region |
| [`aminlocx()`](gradfuncaminlocx.md) | X location of the regional minimum |
| [`aminlocy()`](gradfuncaminlocy.md) | Y location of the regional minimum |
| [`const()`](gradfuncconst.md) | Changes missing or non-missing values |
| [`fndlvl()`](gradfuncfndlvl.md) | Finds the level at which a value occurs |
| [`lterp()`](gradfunclterp.md) | Bilinear interpolation between grids |
| [`max()`](gradfuncmax.md) | Maximum over a grid dimension |
| [`maxloc()`](gradfuncmaxloc.md) | Grid location of the maximum |
| [`min()`](gradfuncmin.md) | Minimum over a grid dimension |
| [`minloc()`](gradfuncminloc.md) | Grid location of the minimum |
| [`maskout()`](gradfuncmaskout.md) | Sets selected values to missing |
| [`smth9()`](gradfuncsmth9.md) | Nine-point smoothing |
| [`tloop()`](gradfunctloop.md) | Reconstructs a time-varying result |
| [`eloop()`](gradfunceloop.md) | Reconstructs an ensemble-varying result |

## Station-data operations

| Function | Description |
| --- | --- |
| [`coll2gr()`](gradfunccoll2gr.md) | Creates a grid from collected station data |
| [`gr2stn()`](gradfuncgr2stn.md) | Grid-to-station interpolation |
| [`oabin()`](gradfuncoabin.md) | Bins station observations into grid cells |
| [`oacres()`](gradfuncoacres.md) | Gridded result representing station data |
| [`s2g1d()`](gradfuncs2g1d.md) | Converts a station time series to a one-dimensional grid |
| [`stnave()`](gradfuncstnave.md) | Time average of station data |
| [`stnmax()`](gradfuncstnmax.md) | Maximum over a station time series |
| [`stnmin()`](gradfuncstnmin.md) | Minimum over a station time series |

## Script and special-purpose functions

| Function | Description |
| --- | --- |
| [`gsfallow()`](gsf.md) | Enables dynamic loading of script functions |
| [`gsfpath()`](gsf.md) | Sets the private search path for script functions |

```{toctree}
:hidden:
:maxdepth: 1

aave() <gradfuncaave>
abs <gradfuncabs>
acos <gradfuncacos>
amax() <gradfuncamax>
amaxlocx() <gradfuncamaxlocx>
amaxlocy() <gradfuncamaxlocy>
amean() <gradfuncamean>
amin() <gradfuncamin>
aminlocx() <gradfuncaminlocx>
aminlocy() <gradfuncaminlocy>
asin <gradfuncasin>
asum() <gradfuncasum>
asumg() <gradfuncasumg>
atan2() <gradfuncatan2>
atot() <gradfuncatot>
ave() <gradfuncave>
cdiff <gradfunccdiff>
coll2gr() <gradfunccoll2gr>
const() <gradfuncconst>
cos <gradfunccos>
eloop <gradfunceloop>
exp <gradfuncexp>
fndlvl() <gradfuncfndlvl>
gint <gradfuncgint>
gr2stn() <gradfuncgr2stn>
hcurl <gradfunchcurl>
hdivg <gradfunchdivg>
if() <gradfuncif>
log <gradfunclog>
log10 <gradfunclog10>
lterp <gradfunclterp>
mag <gradfuncmag>
maskout <gradfuncmaskout>
max() <gradfuncmax>
maxloc() <gradfuncmaxloc>
mean() <gradfuncmean>
min() <gradfuncmin>
minloc() <gradfuncminloc>
oabin() <gradfuncoabin>
oacres() <gradfuncoacres>
pow <gradfuncpow>
s2g1d <gradfuncs2g1d>
scorr() <gradfuncscorr>
sin <gradfuncsin>
skip() <gradfuncskip>
smth9 <gradfuncsmth9>
sqrt <gradfuncsqrt>
sregr() <gradfuncsregr>
stnave <gradfuncstnave>
stnmax <gradfuncstnmax>
stnmin <gradfuncstnmin>
sum() <gradfuncsum>
sumg() <gradfuncsumg>
tan <gradfunctan>
tcorr() <gradfunctcorr>
tloop <gradfunctloop>
tmave() <gradfunctmave>
tregr() <gradfunctregr>
tvrh2q <gradfunctvrh2q>
tvrh2t <gradfunctvrh2t>
vint <gradfuncvint>
```
