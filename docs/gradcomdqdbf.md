---
title: q dbf
---

# **q dbf**

This command lists the contents of a shapefile attribute database.

## Syntax

`q dbf `*`shapefile <delimiter>`*

- *`shapefile`*` ` is the name of the shapefile.\
  *`delimiter`*` ` is an optional argument that provides an alternative string (max 3 characters) to serve as a delimiter between fields (default is a comma).\


### Usage Notes

This command is available with version 2.0.a8. The ` `*`<delimiter>`* option was added with version 2.2.0.

It is not necessary to include the file extension (.dbf) on the end of *`shapefile`*, just the filename root is adequate.

If you put the three shapefile components (\*.shp, \*.shx, and \*.dbf) in the GrADS data directory (pointed to by the <a href="gradcomdgrads.html#env">GADDIR environment variable</a>), then it is not necessary to include the full path in *`shapefile`*.

The first line of output contains a comma-delimited list of the names of all the attributes for each record in the database. Subsequent lines contain the comma-delimited list of all the attribute values for each record.

A companion command, <a href="gradcomdqdbf.html">`q shp`</a>, lists the contents of the shapefile. There is a 1:1 correspondence between elements in the shapefile and records in the database.

Please see the documentation page on <a href="shapefiles.html">shapefiles</a> for more details.

### Examples

```text

ga-> q dbf /home/GIS/climatedivs/divisions 
RECORD#,AREA,PERIMETER,CLIMDIVS_,ST,DIV,NAME,DIVISION_I
0,4.463,11.489,2,MN,2,NORTH CENTRAL,2102
1,0.988,9.344,3,WA,3,PUGET SOUND LOWLANDS,4503
2,3.066,16.450,4,WA,4,E OLYMPIC CASCADE FOOTHILLS,4504
3,3.099,12.733,5,WA,6,EAST SLOPE CASCADES,4506<
.
.
.
385,0.958,5.437,59,OR,4,NORTHERN CASCADES,3504
386,2.712,9.973,49,OR,6,NORTH CENTRAL,3506

ga-> q dbf /home/GIS/climatedivs/divisions {}
RECORD#{}AREA{}PERIMETER{}CLIMDIVS_{}ST{}DIV{}NAME{}DIVISION_I
0{}4.463{}11.489{}2{}MN{}2{}NORTH CENTRAL{}2102
1{}0.988{}9.344{}3{}WA{}3{}PUGET SOUND LOWLANDS{}4503
2{}3.066{}16.450{}4{}WA{}4{}E OLYMPIC CASCADE FOOTHILLS{}4504
3{}3.099{}12.733{}5{}WA{}6{}EAST SLOPE CASCADES{}4506
.
.
.
385{}0.958{}5.437{}59{}OR{}4{}NORTHERN CASCADES{}3504
386{}2.712{}9.973{}49{}OR{}6{}NORTH CENTRAL{}3506


```
