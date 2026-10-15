---
title: GrADS Scripting Language Math Functions
---

# GrADS Scripting Language Math Functions

These functions are available from the GrADS scripting language. Each function
returns its result in the script variable `rc`.

## Trigonometric functions

```text
rc = math_trigfunc(angle[, angle2])
```

| Argument or result | Meaning |
| --- | --- |
| `trigfunc` | `sin`, `cos`, `tan`, `asin`, `acos`, `atan`, `atan2`, `sinh`, `cosh`, `tanh`, `asinh`, `acosh`, or `atanh` |
| `angle` | Angle in radians |
| `angle2` | Second angle, used only by `atan2` |
| `rc` | Result of the trigonometric calculation |

## Numeric functions

| Syntax | Arguments and result |
| --- | --- |
| `rc = math_format(format, num)` | `format` is a C-style floating-point format such as `'%5.2f'`; `rc` is the formatted number. |
| `rc = math_nint(num)` | `num` is a real number; `rc` is `num` rounded to the nearest integer. |
| `rc = math_int(num)` | `num` is a real number; `rc` is the integer part not greater than `num`. |
| `rc = math_log(num)` | `num > 0`; returns the natural logarithm. |
| `rc = math_log10(num)` | `num > 0`; returns the base-10 logarithm. |
| `rc = math_pow(num, exponent)` | Returns `num` raised to `exponent`. |
| `rc = math_sqrt(num)` | Returns the square root of `num`. |
| `rc = math_abs(num)` | Returns the absolute value of `num`. |
| `rc = math_exp(num)` | Returns the exponential, `e` raised to `num`. |
| `rc = math_fmod(num1, num2)` | `num2` must be nonzero; returns the remainder of `num1 / num2`. |
| `rc = math_mod(num1, num2)` | `num2` must be nonzero; returns the integer part of the remainder. |

## String and validation functions

| Syntax | Arguments and result |
| --- | --- |
| `rc = math_strlen(string)` | Returns the length of `string`. |
| `rc = valnum(string)` | Returns `0` if `string` is not a number, `1` if it is an integer, or `2` if it is numeric but not an integer. |
| `rc = wrdpos(string, int)` | Returns the character position at which word `int` begins. |

`string` may be any string variable. For `wrdpos`, `int` must be an integer and
the string normally contains more than one word.

## Usage notes

These functions are available in GrADS version 1.8 and later.

## Example

The following records are from
[`script_math_demo.gs`](_downloads/grads-scripts/script_math_demo.gs):

```text
v = 3.1456
fmt = '%-6.1f'
rc = math_format(fmt, v)
say fmt' of 'v' = 'rc
pi = 3.1415926
d2r = pi / 180
angd = 45
ang = angd * d2r
cos = math_cos(ang)
say 'cos of 'angd' = 'cos
num = '3.1455'
rc = valnum(num)
if (rc = 0) ; say num' is not a number' ; endif
if (rc = 1) ; say num' is an integer' ; endif
if (rc = 2) ; say num' is not an integer' ; endif
v = 3.0
while (v < 4.0)
  rc1 = math_nint(v)
  rc2 = math_int(v)
  print 'nint of 'v' = 'rc1' int of 'v' = 'rc2
  v = v + 0.1
endwhile
pow = math_pow(2, 0.5)
print '2 raised to the power 0.5 = 'pow
num = math_exp(1)
print 'exp(1) = 'num
fmod = math_fmod(5, 2)
print '5 modulo 2 (the remainder when 5 is divided by 2) = 'fmod
s = 'this is a test'
rc = math_strlen(s)
print 'length of the string "'s'" = 'rc
p = 2
rc = wrdpos(s, p)
print 'word 'p' of the string "'s'" starts at character 'rc
```
