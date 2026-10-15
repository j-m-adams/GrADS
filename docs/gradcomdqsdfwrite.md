---
title: q sdfwrite
---

# **q sdfwrite**

`q sdfwrite`

This command returns information on the status of the `sdfwrite` options:

1.  The name of the `sdfwrite` output file
2.  The format of the output file\
3.  The output file's chunk dimensions, if the format is compressed.
4.  The output undef value
5.  Any attributes that have been set by the user

## Usage Notes

This command is available in GrADS v2.0.a3 or higher.

The command to create the self-describing file is <a href="gradcomdsdfwrite.html">`sdfwrite`</a>.

The command to set or change the self-describing output filename is <a href="gradcomdsetsdfwrite.html">`set sdfwrite`</a>.

The command to set attributes for the self-describing file is <a href="gradcomdsetsdfattr.html">`set sdfattr`</a>.

The <a href="gradcomdreset.html">`reset`</a> command will reset the sdfwrite filename to the default and release all the attributes. To do this without resetting all the other user-specified options, use the <a href="gradcomdclear.html">`clear sdfwrite`</a> command.

Please see the documentation on <a href="compression.html">compression</a> for more details.

###
