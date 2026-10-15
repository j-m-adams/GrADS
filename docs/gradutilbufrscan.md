---
title: bufrscan
---

# **bufrscan**

BUFR (Binary Universal Form for the Representation of meteorological data) is a World Meteorological Organization (WMO) standard for storing observational data (aka sequence or in-situ data). BUFR is self-describing data format and can store a large amount of data and metadata in a small amount of disk space by using look-up tables and bit-by-bit packing. Bufrscan is an external GrADS utility that reads BUFR messages and prints out ascii values. 

Individual elements of a BUFR message are uniquely described by three numbers: F, X, and Y. F is a type indicator and may be 0, 1, 2, or 3. X is a class or category indicator and varies between 0 and 63. Y indicates an entry within an X class, and varies between 0 and 255. The F,X,Y trio provides the required decoder table reference, so that the data value may be retrieved from the BUFR element. A group of BUFR elements forms a subset; a group of subsets forms a message; any number of messages may be concatenated together to form a BUFR file.

The syntax for bufrscan is as follows:

`bufrscan [-h] [-d] `*`tablepath filename`*

Where:

- |  |  |
  |----|----|
  | `-h` | Prints the BUFR message headers (default) |
  | `-d` | Prints the BUFR message contents |
  | *`tablepath`* | Directory containing BUFR decoding tables (e.g. /usr/local/grads/lib/tables) |
  | *`filename`* | BUFR message file to be decoded |

When using the -h option to print out the headers, the output from bufrscan will list the F, X, Y values for each element as well as the type of information in the element (given in parentheses). For elements that are type 'numeric' or 'text', a variable name and description are also printed. ([Example header output](_downloads/bufr.sample.headers))

When using the -d option to print out the data, the output from bufrscan will list message number, subset number (in parentheses), replication factor \[in square brackets\], the F,X,Y values, and then the decoded data value. ([Example data output](_downloads/bufr.sample.data))

Many BUFR files also contain a lookup table that was designed to be used with the data in the file. The tables are packed into the BUFR format the same way the data are. If a BUFR file contains a decoding table, the information in that table will override the lookup values in the standard decoding tables underneath the *`tablepath`* directory.

There is a GrADS interface for BUFR, which means that BUFR data can be read directly in their native format and are handled as a GrADS station data set with all the associated display an analysis capabilities. GrADS requires a specially-formatted descriptor file to read BUFR data; the output from bufrscan is used to compose the descriptor file. The document on the <a href="bufrformat.html">BUFR Format</a> has more information on how to generate BUFR descriptor files.
