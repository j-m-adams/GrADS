---
title: Supplemental Libraries for GrADS version 2.2
---

# Supplemental Libraries for GrADS version 2.2

**Recommended installation:** install the conda-forge package, which supplies the required native libraries automatically:

```console
mamba create -n grads -c conda-forge grads
mamba activate grads
```

The source-build instructions below are retained for developers who need to build GrADS themselves.

## Advanced source-build instructions

There are many supplemental libraries that are required to enable various features in the GrADS executable. Building all these libraries from source is not necessarily easy; this page provides some guidance and suggestions that have led to success on COLA's unix systems (64-bit linux running CentOS, and Mac OSX). COLA's objective in building GrADS is to make our binary releases portable, so we strive to build all the libraries from scratch, disabling features GrADS doesn't need, and then link statically when building GrADS. If you are building GrADS from source but not planning to distribute your build, then you may find that many of these libraries are already installed on your system and you can link with them dynamically. In this case, use the --enable-dyn-supplibs option with the GrADS configure script. Please post questions about building from source to the [GrADS Users Forum](https://wetterzentrale.de/grads/docs/index.html). If you have the proper privileges, you may install these anywhere on your system instead of \$HOME, just be sure to change the commands listed in the table below to accomodate your own installation.

To begin, create a top-level directory for the supplemental libraries:

```console
mkdir $HOME/grads/supplibs
setenv SUPPLIBS $HOME/grads/supplibs
```

Create a directory for source tarballs:

```console
mkdir -p $SUPPLIBS/tarfiles
```

Create a directory for unpacked source code:

```console
mkdir -p $SUPPLIBS/src
```

Get source dependencies from your distribution or conda environment. The old COLA FTP archive is no longer maintained; using conda is the supported route for a reproducible build. See the [installation documentation](https://wetterzentrale.de/grads/docs/index.html).

```console
cd $SUPPLIBS/tarfiles
```

The library software creates these installation directories automatically:

```text
$SUPPLIBS/lib
$SUPPLIBS/include
$SUPPLIBS/bin
```

This is necessary for dynamically linking the libraries when creating `libgradspy.so` for the Python interface:

```console
setenv CFLAGS -fPIC
``` 

When you are done, unpack the GrADS source code tarball under \$HOME. Change into the new GrADS directory you just created, and type ./configure. When the configuration is done, it will show a summary of which features have been enabled. Then type 'make install' and look for your executables in the ./bin directory.

Good Luck!!

The `szip` library (version 2.1) is also required by some HDF5 builds. If it is not provided by your operating system or conda environment, build and install it before HDF5:

```console
cd $SUPPLIBS/src
tar xvfz ../tarfiles/szip-2.1.tar.gz
cd szip-2.1
./configure --prefix=$SUPPLIBS
make install
```

## Library summary

| Library | Version | Purpose |
|---|---|---|
| readline | 5.0 | Enables command line editing. [home page](http://tiswww.case.edu/php/chet/readline/rltop.html) |
| ncurses | 5.7 | Required by readline. [home page](http://www.gnu.org/software/ncurses/) |
| zlib | 1.2.8 | General compression library. Required by NetCDF et al. [home page](http://www.zlib.net/) |
| libpng | 1.5.12 | PNG reference library. [home page](http://www.libpng.org/pub/png/libpng.html) |
| jpeg | 6b | Image compression library. [home page](http://www.ijg.org/) |
| gd | 2.0.34 | GD Graphics Library. Requires: zlib, libpng, jpeg [home page](http://www.libgd.org/Main_Page) |
| jasper | 1.900.1 (or 14ubuntu3.2.debian) | For image coding and manipulation [home page](http://www.ece.uvic.ca/~mdadams/jasper/) |
| g2clib | 1.6.0 | Decodes data in GRIB2 format. Requires: zlib, png, jasper [home page](http://www.nco.ncep.noaa.gov/pmb/codes/GRIB2/) |
| udunits | 1.11.7 | Supports units of physical quantities. [home page](http://www.unidata.ucar.edu/software/udunits/) |
| hdf | 4.2r3 | Hierarchical Data Format library, version 4. Requires: zlib, udunits, jpeg, szip [home page](http://hdfgroup.org/products/hdf4/index.html) |
| hdf5 | 1.8.11 | Hierarchical Data Format library, version 5. Requires: zlib [home page](http://www.hdfgroup.org/HDF5/) |
| curl | 7.35.0 (7.19.6 also works) | For enabling OPeNDAP access. [home page](http://curl.haxx.se/) |
| netcdf | 4.3.3 | Network Common Data Form library. Requires hdf5, zlib, szip, curl. [home page](http://www.unidata.ucar.edu/software/netcdf/) |
| tiff | 3.8.2 | Enables handling of image data in the Tag Image File Format. [home page](http://www.libtiff.org/) |
| geotiff | 1.2.5 | Enables handling georeferenced raster imagery. Requires: tiff. [home page](http://geotiff.osgeo.org/) |
| shapelib | 1.2.10 | Enables handling of shapefiles [home page](http://shapelib.maptools.org/) |
| xml2 | 2.9.0 | An XML parser and toolkit. Enables OPeNDAP station data access, also used by Cairo library. [home page](http://xmlsoft.org/) |
| Xrender | 0.9.6 | A helper tool used when compiling applications and libraries. Required for Cairo. [home page](http://cgit.freedesktop.org/xorg/lib/libXrender/) |
| pkgconfig | 0.23 | A helper tool used when compiling applications and libraries. Required for Cairo. [home page](http://pkg-config.freedesktop.org/wiki/) |
| dap | 3.7.8 (use the modified version for newer flavors of unix) | Open-source Project for a Network Data Access Protocol (OPeNDAP). Requires: xml2, curl. [home page](http://opendap.org/index.html) |
| gadap | 2.0 or 2.1 (for newer flavors of unix) | Enables OPeNDAP access of in situ data. Requires dap, curl, and xml2. |
| pixman | 0.34.0 | A pixel manipulation library for X and Cairo. [home page](http://cgit.freedesktop.org/pixman/) |
| freetype | 2.4.10 | A software font engine. Required by Cairo. [home page](http://www.freetype.org/) |
| fontconfig | 2.9.0 | A library for configuring and customizing font access. Required by Cairo. [home page](http://www.freedesktop.org/wiki/Software/fontconfig) |
| cairo | 1.14.10 | A 2D graphics library with support for multiple output devices. Requires pkgconfig, zlib, xml2, libpng, pixman, fontconfig, freetype, and Xrender. [home page](http://www.cairographics.org/) |

## Build instructions

### readline (5.0)

Enables command line editing. [home page](http://tiswww.case.edu/php/chet/readline/rltop.html)

```console
cd $SUPPLIBS/src
tar xvfz ../tarfiles/readline-5.0.tar.gz
cd readline-5.0
./configure --prefix=$SUPPLIBS
make install
```

### ncurses (5.7)

Required by readline.
[home page](http://www.gnu.org/software/ncurses/)

```console
cd $SUPPLIBS/src
tar xvfz ../tarfiles/ncurses-5.7.tar.gz
cd ncurses-5.7
./configure --prefix=$SUPPLIBS --without-ada --with-shared
make install
```

### zlib (1.2.8)

General compression library.
Required by NetCDF et al.
[home page](http://www.zlib.net/)

```console
cd $SUPPLIBS/src
tar xvfz ../tarfiles/zlib-1.2.8.tar.gz
cd zlib-1.2.8
./configure --prefix=$SUPPLIBS
make install
```

### libpng (1.5.12)

PNG reference library.
[home page](http://www.libpng.org/pub/png/libpng.html)

```console
cd $SUPPLIBS/src
tar xvfz ../tarfiles/libpng-1.5.12.tar.gz
cd libpng-1.5.12
./configure --prefix=$SUPPLIBS
make install
```

### jpeg (6b)

Image compression library.
[home page](http://www.ijg.org/)

```console
cd $SUPPLIBS/src
tar xvfz ../tarfiles/jpegsrc.v6b.tar.gz
cd jpeg-6b
./configure --prefix=$SUPPLIBS
make
cp libjpeg.a ../../lib/
cp *.h ../../include/
```

### gd (2.0.34)

GD Graphics Library.
Requires: zlib, libpng, jpeg
[home page](http://www.libgd.org/Main_Page)

```console
cd $SUPPLIBS/src
tar xvfz ../tarfiles/gd-2.0.34.tar.gz
cd gd-2.0.34
./configure --prefix=$SUPPLIBS --with-png=$SUPPLIBS --with-jpeg=$SUPPLIBS
make install
```

### jasper (1.900.1
14ubuntu3.2.debian)

For image coding and manipulation
[home page](http://www.ece.uvic.ca/~mdadams/jasper/)

```console
cd $SUPPLIBS/src
tar xvfz ../tarfiles/jasper-1.900.1-14ubuntu3.2.debian.tar.gz
cd jasper-1.900.1
./configure --prefix=$SUPPLIBS --with-jpeg=$SUPPLIBS
make install
```

### g2clib (1.6.0)

Decodes data in GRIB2 format.
Requires: zlib, png, jasper
[home page](http://www.nco.ncep.noaa.gov/pmb/codes/GRIB2/)

```console
cd $SUPPLIBS/src
tar xvfz ../tarfiles/g2clib-1.6.0.tar.gz
cd g2clib-1.6.0
Note1: There is no configure script in this library, so you must edit the makefile manually. Change the "INC" variable as indicated below. Be sure to write out $SUPPLIBS explicitly:
INC=-I$SUPPLIBS/include -I$SUPPLIBS/include/libpng15
You must also edit the "CFLAGS" variable to include the "-fPIC" option. On some unix servers it may also be necessary to remove the "-m64" and/or "-D__64BIT__" options.
Note2: Version 1.6.0 of the grib2 C library introduced a new output file naming convention that includes a version number. When manually installing in it the $SUPLLIBS/lib directory, rename it to the old (static) filename. It is also necessary to manually install the grib2.h file in the $SUPPLIBS/include directory.
make
/bin/cp -f libg2c_v1.6.0.a $SUPPLIBS/lib/libgrib2c.a
/bin/cp -f grib2.h $SUPPLIBS/include
```

### udunits (1.11.7)

Supports units of physical quantities.
[home page](http://www.unidata.ucar.edu/software/udunits/)

```console
cd $SUPPLIBS/src
tar xvfz ../tarfiles/udunits-1.11.7.tar.gz
cd udunits-1.11.7/src/
./configure --prefix=$SUPPLIBS
make install
```

### hdf (4.2r3)

Hierarchical Data Format library, version 4. Requires: zlib, udunits, jpeg, szip
[home page](http://hdfgroup.org/products/hdf4/index.html)

```console
cd $SUPPLIBS/src
tar xvfz ../tarfiles/HDF4.2r3.tar.gz
cd HDF4.2r3
./configure --prefix=$SUPPLIBS --disable-netcdf --disable-fortran \
--with-zlib=$SUPPLIBS --with-jpeg=$SUPPLIBS
make install
```

### hdf5 (1.8.11)

Hierarchical Data Format library, version 5. Requires: zlib
[home page](http://www.hdfgroup.org/HDF5/)

```console
cd $SUPPLIBS/src
tar xvfz ../tarfiles/hdf5-1.8.11.tar.gz
cd hdf5-1.8.11
./configure --prefix=$SUPPLIBS --disable-fortran --with-zlib=$SUPPLIBS
make install
```

### curl (7.35.0
(7.19.6 also ok))

For enabling OPeNDAP access.
[home page](http://curl.haxx.se/)

```console
cd $SUPPLIBS/src
tar xvfz ../tarfiles/curl-7.35.0.tar.gz
cd curl-7.35.0
Note: When accessing secure (https) opendap servers it is necessary to have the SSL feature of the curl library enabled. Use "--without-ssl" if you do not have the openSSL library installed on your local system.
./configure --prefix=$SUPPLIBS --with-ssl --without-libidn \
--enable-static --disable-ldap
make install
```

### netcdf (4.3.3)

Network Common Data Form library.
Requires hdf5, zlib, szip, curl.
[home page](http://www.unidata.ucar.edu/software/netcdf/)

```console
cd $SUPPLIBS/src
tar xvfz ../tarfiles/netcdf-4.3.3.tar.gz
cd netcdf-4.3.3
Note: before running configure, set the following environment variables (remove the "-lssl" from $LIBS if you did not build curl with SSL enabled):
setenv LIBS "-lm -ldl -lcurl -lssl -lrt -lz"
setenv LDFLAGS -L$SUPPLIBS/lib
setenv CPPFLAGS -I$SUPPLIBS/include
./configure --prefix=$SUPPLIBS --enable-netcdf-4 --enable-dap
make install
Note: After the library is built, you can unset the environment variables:
unsetenv LIBS
unsetenv LDFLAGS
unsetenv CPPFLAGS
```

### tiff (3.8.2)

Enables handling of image data in the Tag Image File Format.
[home page](http://www.libtiff.org/)

```console
cd $SUPPLIBS/src
tar xvfz ../tarfiles/tiff-3.8.2.tar.gz
cd tiff-3.8.2
./configure --prefix=$SUPPLIBS
make install
```

### geotiff (1.2.5)

Enables handling georeferenced raster imagery. Requires: tiff.
[home page](http://geotiff.osgeo.org/)

```console
cd $SUPPLIBS/src
tar xvfz ../tarfiles/libgeotiff-1.2.5.tar.gz
cd libgeotiff-1.2.5
./configure --prefix=$SUPPLIBS --enable-incode-epsg \
--enable-static --with-libtiff=$SUPPLIBS
make
make install
```

### shapelib (1.2.10)

Enables handling of shapefiles
[home page](http://shapelib.maptools.org/)

```console
cd $SUPPLIBS/src
tar xvfz ../tarfiles/shapelib-1.2.10.tar.gz
cd shapelib-1.2.10
Note: There is no configure script, just a Makefile. Edit the makefile to use the -fPIC option with gcc. Change "CFLAGS" as indicated below:
CFLAGS = -g -fPIC
Also change "-g -O2" to "-g -fPIC -O2" when it occurs instead of $(CFLAGS)
make all lib
Installation to $SUPPLIBS is done manually. The utilities are copied to the $SUPPLIBS/bin directory in case they might be useful to the user -- GrADS doesn't explicitly need them.
/bin/cp -f ./.libs/libshp.a $SUPPLIBS/lib
/bin/cp -f shapefil.h $SUPPLIBS/include
/bin/cp -f shpcreate shpadd shpdump shprewind dbfcreate dbfadd dbfdump shptest $SUPPLIBS/bin
```

### xml2 (2.9.0)

An XML parser and toolkit.
Enables OPeNDAP station data access, also used by Cairo library.
[home page](http://xmlsoft.org/)

```console
cd $SUPPLIBS/src
tar xvfz ../tarfiles/libxml2-2.9.0.tar.gz
cd libxml2-2.9.0
./configure --prefix=$SUPPLIBS --with-zlib=$SUPPLIBS --without-threads \
--without-iconv --without-iso8859x --without-lzma
make install
```

### Xrender (0.9.6)

A helper tool used when compiling applications and libraries. Required for Cairo.
[home page](http://cgit.freedesktop.org/xorg/lib/libXrender/)

```console
cd $SUPPLIBS/src
tar xvfz ../tarfiles/libXrender-0.9.6.tar.gz
cd libXrender-0.9.6
./configure --prefix=$SUPPLIBS
make install
```

### pkgconfig (0.23)

A helper tool used when compiling applications and libraries. Required for Cairo.
[home page](http://pkg-config.freedesktop.org/wiki/)

```console
cd $SUPPLIBS/src
tar xvfz ../tarfiles/pkgconfig-0.23.tar.gz
cd pkg-config-0.23
./configure --prefix=$SUPPLIBS
make install
Note: These environment variables must be set AFTER pkg-config is built:
setenv PKG_CONFIG $SUPPLIBS/bin/pkg-config
setenv PKG_CONFIG_PATH $SUPPLIBS/lib/pkgconfig
```

### dap (3.7.8 (use the modified version for newer flavors of unix))

Open-source Project for a Network Data Access Protocol (OPeNDAP).
Requires: xml2, curl.
[home page](http://opendap.org/index.html)

```console
cd $SUPPLIBS/src
tar xvfz ../tarfiles/libdap-3.7.8-modified.tar.gz
cd libdap-3.7.8-modified
setenv CPPFLAGS -I$SUPPLIBS/include
./configure --prefix=$SUPPLIBS
make install
```

### gadap (2.0 or 2.1 (for newer flavors of unix))

Enables OPeNDAP access of
in situ data.
Requires dap, curl, and xml2.

```console
cd $SUPPLIBS/src
tar xvfz ../tarfiles/gadap-2.1.tar.gz
cd gadap-2.1
N.B. You will need to make sure $SUPPLIBS/bin is in your path so the configure script can find the utility dap-config, which is part of the dap library package
setenv PATH $SUPPLIBS/bin:$PATH
setenv CPPFLAGS -I$SUPPLIBS/include
./configure --prefix=$SUPPLIBS
make install
```

### pixman (0.34.0)

A pixel manipulation library for
X and Cairo.
[home page](http://cgit.freedesktop.org/pixman/)

```console
cd $SUPPLIBS/src
tar xvfz ../tarfiles/pixman-0.34.0.tar.gz
cd pixman-0.34.0
./configure --prefix=$SUPPLIBS
make install
```

### freetype (2.4.10)

A software font engine. Required by Cairo.
[home page](http://www.freetype.org/)

```console
cd $SUPPLIBS/src
tar xvfz tarfiles/freetype-2.4.10.tar.gz
cd freetype-2.4.10
./configure --prefix=$SUPPLIBS --with-zlib=$SUPPLIBS \
--without-fsspec --without-fsref --without-ats --without-bzip2 \
--without-quickdraw-toolbox --without-quickdraw-carbon
make install
```

### fontconfig (2.9.0)

A library for configuring and customizing font access. Required by Cairo.
[home page](http://www.freedesktop.org/wiki/Software/fontconfig)

```console
cd $SUPPLIBS/src
tar xvfz tarfiles/fontconfig-2.9.0.tar.gz
cd fontconfig-2.9.0
./configure --prefix=$SUPPLIBS --enable-libxml2 \
--with-freetype-config=$SUPPLIBS/bin/freetype-config \
--with-add-fonts=/Library/Fonts,/System/Library/Fonts (for mac)
--with-add-fonts=/usr/share/X11/fonts,/usr/share/fonts (for unix)
N.B. After configuration, edit config.h to set USE_ICONV = 0
I don't know of another way to tell it not to use libiconv.
make install
```

### cairo (1.14.10)

A 2D graphics library with support for multiple output devices. Requires pkgconfig, zlib, xml2, libpng, pixman, fontconfig, freetype, and Xrender.
[home page](http://www.cairographics.org/)

```console
cd $HOME/supplibs/src
tar xvf tarfiles/cairo-1.14.10.tar.gz
mkdir cairo
cd cairo-1.14.10
./configure --prefix=$SUPPLIBS \
--enable-xlib=yes \
--enable-xml=yes \
--enable-fc=yes \
--enable-ft=yes \
--enable-xlib-xrender=yes \
--enable-pthread=yes \
--enable-xcb=no \
--enable-qt=no \
--enable-quartz=no \
--enable-win32=no \
--enable-skia=no \
--enable-os2=no \
--enable-beos=no \
--enable-drm=no \
--enable-gl=no
make install
```

