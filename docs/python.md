---
title: The GrADS Python interface (GradsPy)
---

# The GrADS Python interface (GradsPy)

GrADS provides a C-based Python extension named **GradsPy**. The conda-forge
distribution provides the native GrADS program and the Python wrapper as
separate packages.

## Installation

Create an environment with GrADS, activate it, and install the wrapper:

```console
mamba create -n grads -c conda-forge grads
mamba activate grads
mamba install -c conda-forge gradspy
```

Verify the installation:

```console
python -c "import gradspy; print(gradspy)"
```

The conda packages provide the native libraries and runtime integration. No
manual compilation or library-path exports are required. The activation hook
configures `GAGPY`, `GADDIR`, and `GAUDPT`; do not set these variables manually
when using the conda-forge packages.

## GradsPy methods

After importing GradsPy, use the following methods to interact with GrADS:

| Method | Purpose |
| --- | --- |
| `start` | Start GrADS with optional switches and arguments. |
| `cmd` | Execute a GrADS command and return its textual output. |
| `result` | Evaluate a GrADS expression and return its data and metadata. |

### `start`

Call `start` once to launch GrADS. Most space-delimited GrADS arguments become
comma-separated Python arguments. When `-c` is used with a script that takes
arguments, put the script name and its arguments in one quoted argument.

```python
import gradspy

gradspy.start()
gradspy.start("-lb")
gradspy.start("-lc", "open /data/samples/model.ctl")
gradspy.start("-a", "1.0", "-g", "800x800+60+0", "-d", "X11")
gradspy.start("-lb", "-c", "scriptname.gs arg1 arg2")
```

### `cmd`

Use `cmd` to issue any GrADS command. It returns the text that GrADS would
normally write to its command window. Store the returned text when it needs to
be parsed or formatted:

```python
a = gradspy.cmd("q file")
print(a)

b = gradspy.cmd("q dims")
print(b)
```

### `result`

Use `result` to evaluate a GrADS expression. It returns a tuple containing the
resulting NumPy grid and its coordinate and metadata arrays:

```python
rt = gradspy.result("ave(tsfc,t=1,t=12)")
```

The tuple contains seven elements:

| Index | Contents |
| ---: | --- |
| `0` | Return code. A negative value indicates an error; otherwise it is the number of varying dimensions (the rank). |
| `1` | Two-dimensional NumPy data array. Missing values are represented by `NaN`. |
| `2` | One-dimensional longitude coordinates (`NaN` if X is not varying). |
| `3` | One-dimensional latitude coordinates (`NaN` if Y is not varying). |
| `4` | One-dimensional level coordinates (`NaN` if Z is not varying). |
| `5` | Fourteen integer metadata values describing dimension sizes and time/ensemble coordinates. |
| `6` | Six floating-point metadata values describing the X, Y, and Z origins and increments. |

### Integer metadata (`rt[5]`)

| Position | Meaning |
| ---: | --- |
| `0` | X (longitude) size; `1` if X is not varying |
| `1` | Y (latitude) size; `1` if Y is not varying |
| `2` | Z (level) size; `1` if Z is not varying |
| `3` | T (time) size; `1` if T is not varying |
| `4` | E (ensemble) size; `1` if E is not varying |
| `5` | T start year |
| `6` | T start month |
| `7` | T start day |
| `8` | T start hour |
| `9` | T start minute |
| `10` | T increment |
| `11` | T increment type: `0` for months, `1` for minutes |
| `12` | Calendar type: `0` for normal, `1` for 365-day |
| `13` | E start value; the E increment is always `1` |

### Floating-point metadata (`rt[6]`)

| Position | Meaning |
| ---: | --- |
| `0` | X start value when X is linear |
| `1` | X increment; negative when X is non-linear |
| `2` | Y start value when Y is linear |
| `3` | Y increment; negative when Y is non-linear |
| `4` | Z start value when Z is linear |
| `5` | Z increment; negative when Z is non-linear |

## Usage notes

When an expression produces a two-dimensional latitude/longitude grid, GrADS
orders the data as `(i, j)`—X/longitude first and Y/latitude second. Python uses
the opposite array convention, so the returned NumPy array is ordered as
`(j, i)`—latitude first and longitude second. The same reversal applies to any
two-dimensional result: a GrADS `(i, j)` array is returned to Python as `(j, i)`.
