---
title: tvrh2q
---

# **tvrh2q**

`tvrh2q(`*`tvexpr,rhexpr`*`)`

Given virtual temperature and relative humidty, `tvrh2q` returns specific humidity, q, in g/g. Specifically:

*`tvexpr`*     A GrADS expression that represents virtual temperature, in Kelvin.\
*`rhexpr`*     A GrADS expression that represents relative humidty, in percent (values from 0 to 100).

This function works only on gridded data.

## Usage Notes

1.  The conversion formula requires a pressure in mb. `tvrh2q` assumes that the Z coordinate system is pressure in mb. If Z is a varying dimension, the pressure valid at each grid point is used. When Z is a fixed dimension, the Z value from the current dimension environment is used.

    Note that it is possible to provide values from an incorrect pressure level by overriding the current dimension environment:

    ` `

    set lev 500

    d tvrh2q(tv(lev=850),rh(lev=850))

    In this case, the `tvrh2q` function assumes a pressure of 500mb, which is the current setting for the Z dimension environment. However, *`tvexpr`* and *`rhexpr`* are providing data from the 850mb level, so the function will produce incorrect results.

### Examples
