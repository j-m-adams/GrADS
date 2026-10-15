---
title: Gradcomdstat
---

# Gradcomdstat

`set gxout stat`\
`d `\

sends output to the terminal as opposed to a plot or data output (e.g., `set fwrite out.dat ; set gxout fwrite; d rh`). Or the output goes to the script variable `result` which can be parsed inside a script (see the `corr.gs` GrADS script)

The output allows many statistical calculations to be made. Here's an example of opening up a global model file and looking at the 1000 mb relative humidity, statistically,

- `ga-> set gxout stat`\
  `ga-> d rh`\
  `Data Type = grid`\
  `Dimensions = 0 1`\
  `I Dimension = 1 to 145`\
  `J Dimension = 1 to 73`\
  `Sizes = 145 73 10585`\
  `Undef value = 1e+20`\
  `Undef count = 0 Valid count = 10585`\
  `Min, Max = 0.0610352 100.061`\
  `Stats(sum,sumsqr,n): 787381 6.35439e+07 10585`\
  `Stats(sum,sumsqr)/n: 74.3865 6003.2`\
  `Stats(sum,sumsqr)/(n-1): 74.3935 6003.77`\
  `Stats(sigma,var)(n): 21.6761 469.854`\
  `Stats(sigma,var)(n-1): 21.6771 469.898`\
  `Cmin, cmax, cint = 10 100 10`

Let's break it down:

- `Data Type = grid` ----- you have a grid

  `Dimensions = 0 1` ----- the dimension type for the variable\

  - `0` - lon\
    `1` - lat\
    `2` - lev\
    `3` - time

  \
  `1` - not varying

  `I Dimension = 1 to 145` ------ obvious

  `J Dimension = 1 to 73`

  `Sizes = 145 73 10585` ------- `10585` is 145\*73 or total number of points

  `Undef value = 1e+20` ------- undefined value

  `Undef count = 0 Valid count = 10585` ----- \# of defined and undefined points in the grid. Remember that if GrADS can't find any data it returns undefs. This is useful for checking if you have any data, `Valid count = 0` means no...

  `Min, Max = 0.0610352 100.061` ---- UHHH OHHHH! we have slight supersaturation..

  `Stats(sum,sumsqr,n): 787381 6.35439e+07 10585` - This should be fairly obvious, sum = the simple sum of all **defined** points.

  `sumsqr` = sum of, in this case, `rh*rh` and `10585` is `n`.

  `Stats(sum,sumsqr)/n: 74.3865 6003.2` - Divide by `n` for convenience, the first number is the "biased" mean...

  `Stats(sum,sumsqr)/(n-1): 74.3935 6003.77` - the so called `unbiased` mean (remove 1 degree of freedom), etc.

  `Stats(sigma,var)(n): 21.6761 469.854` - the standard deviation and variance "biased" (`n`)

  `Stats(sigma,var)(n-1): 21.6771 469.898` - the standard deviation and variance "unbiased" (`n-1`)

  `Cmin, cmax, cint = 10 100 10` - What GrADS will use when contouring.

**NOTE**: This works for both `gridded` and **station** data
