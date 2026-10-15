---
title: About GrADS Gridded Data Sets
---

# About GrADS Gridded Data Sets

This section describes GrADS gridded data sets -- their structure andformat, how to create them, and how to instruct GrADS to interpretthem properly. Here are some quick links for skipping through thissection:

- <a href="aboutgriddeddata.html#introduction">Introduction</a>
- <a href="aboutgriddeddata.html#descriptor">The Data Descriptor File</a>
- <a href="aboutgriddeddata.html#structure">Structure of a Gridded Binary Data File</a>
- <a href="aboutgriddeddata.html#formats">Binary Formats</a>
- <a href="aboutgriddeddata.html#create">Creating Data Files</a>

------------------------------------------------------------------------

(introduction)=
## Introduction

In GrADS, the raw binary data and the meta data (information about thebinary data) are stored in separate files. The meta data file containsa complete description of the binary data as well as instructions forGrADS on where to find the data and how to read it. The binary datafile is purely data with no space or time identifiers. The meta datafile is the one you open in GrADS -- it is called the **datadescriptor file**. The data descriptor file has a default file extension of `.ctl` and is therefore also referred to as a **control file**.

`ga->` [open](gradcomdopen) `*filename*.ctl`

(descriptor)=
## The Data Descriptor File

The data descriptor file contains a complete description of the binarydata as well as instructions for GrADS on where to find the data andhow to read it. The descriptor file is an ascii file that can becreated easily with a text editor. The general contents of a griddeddata descriptor file are as follows:

- Filename for the binary data
- Missing or undefined data value
- Mapping between grid coordinates and world coordinates
- Description of variables in the binary data set

The individual components of data descriptor files are discussed indetail in the section <a href="descriptorfile.html">Elements of a DataDescriptor File</a>.

The data descriptor file is free format, which means the components of each record (line of text) are blank delimited. Leading blanks at the beginning of each record are removed before parsing. Comment records must start with an asterisk (\*). Individual records may not be more than 255 characters long. Here is an example of a basic data descriptor file:

```text

DSET ^gridded_data_sample.dat 
TITLE Gridded Data Sample
UNDEF -9.99E33
XDEF 180 LINEAR 0.0  2.0 
YDEF 90 LINEAR -90  2.0 
ZDEF 10 LEVELS 1000 850 700 500 400 300 250 200 150 100 
TDEF 4 LINEAR 0Z10apr1991 12hr 
VARS 4
slp     0  99  sea level pressure 
hgt    10  99  heights 
temp   10  99  temperature 
shum    6  99  specific humidity
ENDVARS

```

In this example, the binary data set is named`gridded_data_sample.dat` and is located in the samedirectory as the descriptor file. This is specified by the caret (^) infront of the data filename. The undefined or missing data value is-9.99e33, there are 180 grid points in the X direction, 90 grid pointsin the Y direction, 10 vertical levels, 4 time steps, and 4variables. The variable "slp" is a surface variable -- it has novertical levels, but is assigned a default vertical coordinate ofZ=1. The variables "hgt" and "temp" have 10 vertical levels, and thevariable "shum" has 6 vertical levels (the first six listed, 1000 to300).

(structure)=
## Structure of a Gridded Binary Data File

The binary data file is purely data with no space or timeidentifiers. The data descriptor specifies the data's grid dimensions,but it is up to the user to make sure that the binary data have beenwritten to file in the proper order so GrADS will interpret themcorrectly.

GrADS views gridded data sets as multi-dimensional arrays varying inlongitude, latitude, vertical level, variable, and time. In version 2.0, a <a href="ensembles.html">fifth grid dimension</a> was added. The fifth grid dimension is assumed to be used for ensembles, and so it is given the name E (or ens), but because it is generally implemented, it may be used for other grid dimensions such as EOFs. The default size of the E dimension is 1 -- if no E dimension exists, it is not necessary to explicity declare it the descriptor file.

It is helpful to think of a gridded binary data file as a sequence of "building blocks", where each building block is a horizonal grid of data varying in the X and Y dimensions. The first dimension (X) always varies from west to east; the second dimension (Y) varies from south to north (by default). One horizontal grid represents a particular variable at a particular height and time. Each horizontal grid in a GrADS binary data file must be the samesize. If you have two variables with different horizontal grids,you must create two separate data sets.

The structure of a 3-D, 4-D, or 5-D data set is determined by theorder in which the horizonal grids are written to file. The buildingblocks are stacked in a sequence according to dimension. The sequencegoes in the following order starting from the fastest varyingdimension to the slowest varying dimension: longitude (X), latitude(Y), vertical level (Z), variable (VAR), time (T), and ensemble (E).

For example, suppose you want to create a 4-D binary data setcontaining four variables. The horizonal grids would be written to thedata set in the following order:

```text

Time 1, Variable 1, Each vertical level from bottom to top
Time 1, Variable 2, Each vertical level from bottom to top
Time 1, Variable 3, Each vertical level from bottom to top
Time 1, Variable 4, Each vertical level from bottom to top

Time 2, Variable 1, Each vertical level from bottom to top
Time 2, Variable 2, Each vertical level from bottom to top
Time 2, Variable 3, Each vertical level from bottom to top
Time 2, Variable 4, Each vertical level from bottom to top

etc.

```

5-D ensemble data sets are created by concatenating 4-D data sets together -- the ensemble dimension varies outside of all the others. To extend the previous example, suppose you are creating a data set that has 3 ensembles (but only 2 variables) :

```text

Ensemble 1, Time 1, Variable 1, Each vertical level from bottom to top
Ensemble 1, Time 1, Variable 2, Each vertical level from bottom to top
Ensemble 1, Time 2, Variable 1, Each vertical level from bottom to top
Ensemble 1, Time 2, Variable 2, Each vertical level from bottom to top
            <...>
Ensemble 1, Time N, Variable 1, Each vertical level from bottom to top
Ensemble 1, Time N, Variable 2, Each vertical level from bottom to top
Ensemble 2, Time 1, Variable 1, Each vertical level from bottom to top
Ensemble 2, Time 1, Variable 2, Each vertical level from bottom to top
Ensemble 2, Time 2, Variable 1, Each vertical level from bottom to top
Ensemble 2, Time 2, Variable 2, Each vertical level from bottom to top
            <...>
Ensemble 2, Time N, Variable 1, Each vertical level from bottom to top
Ensemble 2, Time N, Variable 2, Each vertical level from bottom to top
Ensemble 3, Time 1, Variable 1, Each vertical level from bottom to top
Ensemble 3, Time 1, Variable 2, Each vertical level from bottom to top
Ensemble 3, Time 2, Variable 1, Each vertical level from bottom to top
Ensemble 3, Time 2, Variable 2, Each vertical level from bottom to top
            <...>
Ensemble 3, Time N, Variable 1, Each vertical level from bottom to top
Ensemble 3, Time N, Variable 2, Each vertical level from bottom to top

```

(formats)=
## Binary Formats

GrADS can read binary data that are formatted with or withoutFORTRAN record length headers. Files containing record length headersare called "sequential" and those without embedded record lengthinformation are called "direct access" or "stream" files. Unless otherwise specified, GrADS will assume the data file does not contain the record length headers.

GrADS can also directly read GRIB formatted data -- one of GrADS most powerful and unique features! See the section on Creating Data Descriptor Files for GRIB Data for more information.

A third category of data formats that GrADS can read are"self-describing files" such as NetCDF or HDF-SDS. For moreinformation, see the references pages for reading NetCDF or HDF-SDS files.

(create)=
## Creating Data Files

The default format for GrADS gridded binary data files is "stream" or"direct access". If you want to read FORTRAN "sequential" unformattedbinary data files, you must include the following additional record inthe data descriptor file:

- <a href="descriptorfile.html#OPTIONS">`OPTIONS`</a>` sequential`

Following are three examples of how to create gridded binary datafiles with simple FORTRAN programs.

1.  Suppose you have U and V wind components in 4-dimensions (X, Y, Z, and T)and you want to write them out in so they can be viewed in GrADS. TheFORTRAN code might look something like this:

```fortran

parameter (ni=144,nj=91,nk=8,nt=4) 
dimension u(ni,nj,nk),v(ni,nj,nk),dum(ni,nj) 
do n=1,nk 
   call load(u,ni,nj,nk,n,dum)
   write(10) dum 
end do 
do n=1,nk 
   call load(v,ni,nj,nk,n,dum)
   write(10) dum 
end do 

subroutine load(a,ni,nj,nk,n,dum)
dimension a(ni,nj,nk),dum(ni,nj) 
do i=1,ni 
   do j=1,nj 
   dum(i,j)=a(i,j,n) 
end do 
end do 
return

```

The data descriptor file would look something like:
```text

DSET ^model.dat 
TITLE Sample Model Data 
UNDEF 0.10000E+16 
XDEF 144 linear   0 2.5 
YDEF 91 linear -90 2.0 
ZDEF 8 levels 1000 900 800 700 500 300 100 50
TDEF 4 linear 00z01apr85 6hr
VARS 2 
   u 8 99 U component 
   v 8 99 V component 
ENDVARS

```

2.  This simple example write out one variable:
```fortran

   REAL  Z(72,46,16)
   ....
   OPEN(8,FILE='grads.dat',FORM='UNFORMATTED',
 & ACCESS='DIRECT',RECL=72*46)
   ....
   IREC=1 
   DO 10 I=1,16
     WRITE (8,REC=IREC) ((Z(J,K,I),J=1,72),K=1,46)
     IREC=IREC+1
10 CONTINUE

```

3.  Another simple sample might be:

```fortran

   REAL X(100) 
   DO 10 I=1,100 
     X(I)=I  
10 CONTINUE 
   OPEN (8,FILE='samp.dat',FORM='UNFORMATTED',ACCESS='DIRECT',
  &RECL=100) 
   WRITE (8,REC=1) X 
   STOP 
   END

```

The associated descriptor file:

```text

DSET samp.dat 
TITLE Sample Data Set 
UNDEF -9.99E33 
XDEF 100 LINEAR 1 1 
YDEF 1 LINEAR 1 1 
ZDEF 1 LINEAR 1 1 
TDEF 1 LINEAR 1JAN2000 1DY 
VARS 1 
x  0  99  100 Data Points 
ENDVARS

```

Once created, you can use this data set to experiment with GrADSdata functions, such as:

[`display`](gradcomddisplay) [`sin`](gradfuncsin) `(x/50)`
