---
title: set sdfattr
---

GrADS Command: set sdfattr

# **set sdfattr**

`set sdfattr `*`varname attribute_type attribute_name attribute_value`*

Sets attribute metadata to be included in the output file created with the <a href="gradcomdsdfwrite.html">`sdfwrite`</a> command.

- *`varname`*`           `May be set to "global", "longitude", "latitude", "level", "time", "ensemble", or\
  `                  `a defined variable name\
  *`attribute_type`*       May be one of five data type types (valid aliases in parentheses):\
  `                  `"char" (String, Str, Url)\
  `                  `"short" (Int16, UInt16)\
  `                  `"int" (long, Int32, UInt32)\
  `                  `"float" (Float32)\
  `                  `"double" (Float64).\
  *`attribute_name`*`    `May be any single word or string with no spaces (e.g.: "units", "long_name")\
  *`attribute_value`*`   `May be any string as long as the length of the entire entry does not exceed 512 characters.\
                                      For numeric types, may be a list of comma-delimited numerals.

## Usage Notes

This command is available in GrADS v2.0.a3 or higher.

The <a href="gradcomdreset.html">`reset`</a> command will release all the attributes from memory. To do this without resetting all the other user-specified options, use the <a href="gradcomdclear.html">`clear sdfwrite`</a> command.

### Examples

`set sdfattr precip String units mm/day`\
`set sdfattr precip String long_name Daily Precipiation `\
`set sdfattr precip double actual_range 0,57.5`
