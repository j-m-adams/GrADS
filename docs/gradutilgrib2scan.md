---
title: grib2scan
---

# **grib2scan**

The `grib2scan` utility is used for finding out what is inside a GRIB2 data file. It scans each record in the file, and prints out relevant information about the data in the record. The command sytax is:

- `grib2scan [-v] `*`fname`*

Where *`fname`* is the input GRIB2 file name, and the `-v` option is used for verbose output.

To assist in the interpretation of the grib2scan output, here are some links to GRIB2 documentation: [<br />
WMO](http://www.wmo.int/pages/prog/www/WMOCodes/GRIB.html) (the official source)[<br />
NCEP](http://www.nco.ncep.noaa.gov/pmb/docs/grib2/grib2_doc.shtml) (unofficial, but with a user-friendly web interface).

A GRIB2 file is a collection of records, each with a complete set of metadata to describe the data contained in the record. A GRIB2 record has 9 sections, some of which may occur more than once. The output from grib2scan is printed out as it scans each section of the record. Below are examples of the grib2scan output from two different GRIB2 records. An explanation of the output follows.

`Record 1 starts at 0 of length ``17242`\
`  ``Discipline=0`` (Meteorological)`\
`  Reference Time`` = 2007-10-09 00:00:00 ``Start of Forecast`\
` Field 1`\
`  ``GDT=0`` (Lat/Lon) nx*ny=10512`\
`   XDEF 144 linear 0.000000 2.500000`\
`   YDEF 73 linear 90.000000 2.500000`\
`  ``PDT=1`` Forecast Time = 0 Hour`\
`  Parameter: ``disc,cat,num`` = 0,3,5`\
`  Level: ``ltype,lval`` = 100,20000 (sf,sval = 0 20000)`\
`  Ens: ``type,pert`` = 3,1 (total=10)`\
`  ``DRT=40`` (Grid Point Data - JPEG2000 Compression) (Lossless)`\
`Record 2 starts at 17242 of length ``110483`\
`  ``Discipline=0`` (Meteorological)`\
`  ``Reference Time`` = 2007-10-09 00:00:00 ``Start of Forecast`\
` Field 1`\
`  ``GDT=0`` (Lat/Lon) nx*ny=259920`\
`   XDEF 720 linear 0.000000 0.500000    `\
`   YDEF 361 linear 90.000000 0.500000`\
`  ``PDT=8`` (6 Hour Average) EndTime = 2007-10-09 06:00`\
`  Parameter: ``disc,cat,num,sp`` = 0,1,7,0`\
`  Level: ``ltype,lval`` = 1,0 (sf,sval = 0 0)`\
`  ``DRT=40`` (Grid Point Data - JPEG2000 Compression) (Lossless) `

Section 0, the Indicator Section, contains info grib2scan uses to confirm it is looking at a GRIB2 file. This section also contains the Discipline, as well as the size of the entire record.

Section 1, the Identification Section, contains characteristics that apply to all data in the record. Sections 0 and 1 are the only ones that may not be repeated in a GRIB2 record. From section 1 we get the Reference Time printed in a YYYY-MM-DD HH:MM:SS format, plus the significance of the reference time.

Section 3, the Grid Definition Section, contains the definition of the grid surface and geometry of the data values in the grid. The Grid Definition Template (GDT) value indicates the grid type -- in Record 1, the data are on a Lat/Lon grid at 2.5-degree resolution; Record 2 has a 0.5-degree Lat/Lon grid. For some GDT values, grib2scan in verbose mode will print out helpful information for creating the XDEF and YDEF or PDEF entries for the descriptor file.

Section 4, the Product Definition Section, contains a lot of essential information about the nature of the data -- what the variable is called, its units, its vertical level, its valid time, whether it is part of an ensemble set, and whether it is an instantaneous value or statistically processed in some way. The Product Definition Template (PDT) value tells grib2scan where to get the parameter category and number, which, along with the discipline, are required codes to uniquely identify the variable in the descriptor file. These three numbers, the discipline, category, and number, are printed out in comma-delimed format, so you can cut and paste them directly into your descriptor file. In Record 1, the parameter is "Discipline 0: Meteorological products, Parameter Category 3: Mass, Parameter Number 5: Geopotential Height". If the parameter has been statistically processed (i.e. it is an average or an accumulation over a period of time), then a fourth number is required by GrADS to indicate the kind of statistical process used to generate the parameter: the discipline, category, number, and statistical process must go into the descriptor file. In Record 2, the parameter is "Discipline 0: Meterological products, Parameter Category 1: Moisture, Parameter Number 7: Precipitation rate, Statistical Process 0: Average (6 hour)". Section 4 also give the parameter's level type and value -- these comma-delimited numbers can also be copied from the grib2scan output into your descriptor file. In the first example, the level type is pressure, and the level value is 20000 Pascals (or 200 mb). Note that if a parameter occurs on more than one pressure level, you should use ZDEF to list the pressure level values and omit the level value in the variable declaration. Pressure values are converted back to millibars internally if you use the OPTIONS keyword "pascals". In Record 2, the level type is the ground or water surface and the value is 0. Finally, if it is relevant, ensemble metadata also appears in section 4. Ensemble metadata is only relevant in Record 1, and appears as an ensemble type and perturbation number. Parameters that are derived from all ensemble members only require one code, a number that indicates the type of derived forecast (e.g. ensemble mean). These ensemble codes belong in the exanded form of the EDEF declaration.

Section 5, the Data Representation Section, contains information about how the data are represented. The Data Representation Template (DRT) number is not required in a GrADS descriptor file, but because it is sometimes useful to know how the data are formatted in the GRIB2 file, it is included in the grib2scan output.
