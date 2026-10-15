---
title: set undef
---

# **set undef**

This command allows the user to set the undefined data value for all forms of GrADS output. This command controls the undefined values printed to screen with the 'gxout stat' and the 'gxout print' output, as well as the undefined values in fwrite, sdfwrite, and geotiff files.

The output undef value will not necessarily be the same as the native undef value of the data set -- the default value is -9.99e8. The user can easily change the output undef value to match the undef value of the default file or a specific open file.\

## Syntax

`set undef `*`value`*`    `(sets output undef value to *value*)\
`set undef file `*`n`*`   `(copies undef value from file *n*)\
`set undef dfile    ` (copies undef value from default file)

### Usage Notes

This is a new feature with GraDS version 2.0.a6, that changes the default behavior of GrADS!\
By default, the output undef value will be -9.99e8 instead of the undef value given in the data descriptor file. You may easily revert to the old behavior by using the 'set undef file n' option.

You can find out what the current output undef value is with the `'`<a href="gradcomdquery.html">`query undef`</a>`'` command.

The <a href="gradcomdreinit.html">`reinit`</a> command will return the output undef value to the default (-9.99e8); the <a href="gradcomdreset.html">`reset`</a> command leaves it unchanged.

If you issue the '`set undef dfile`' command, the output undef value will be copied from the default file's undef value. If you subsequently change the default file number, the output undef value will not change. You must issue '`set undef dfile`' again if you wish the output undef value to be the same as the new default file.

###
