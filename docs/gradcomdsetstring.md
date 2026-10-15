---
title: set string
---

# **set string**

`set string `*`color <justification <thickness<rotation>>>`*

Sets `string` drawing attributes. *`Color`* isas described above. *`Justification`* is the stringjustification, or how the string isplotted with respect to the x, y position given in the `drawstring` command. Refer to the following picture for the appropriatecodes:


```text

           tl            tc              tr          tl - top left

            +-------------+--------------+           tc - center top

            |                            |           tr - right top

          l +             + c            + r              etc.

            |                            |

            +-------------+--------------+

           bl             bc             br

```

The *`rotation`* option specifies the desired stringrotation in degrees. When rotated, the center of rotation is the*`justification`* point. Rotation is counter-clockwise.

## Usage Notes

### Examples
