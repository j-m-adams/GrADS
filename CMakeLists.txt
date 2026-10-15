cmake_minimum_required(VERSION 3.24)
project(GrADS VERSION 2.3 LANGUAGES C)

if(NOT WIN32)
  message(FATAL_ERROR "The CMake build currently targets native Windows; use configure/make on Unix")
endif()

include(GNUInstallDirs)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

set(GRADS_VERSION "${PROJECT_VERSION}")
set(GRADS_GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
file(MAKE_DIRECTORY "${GRADS_GENERATED_DIR}")
configure_file(cmake/config.h.in "${GRADS_GENERATED_DIR}/config.h" @ONLY)
file(WRITE "${GRADS_GENERATED_DIR}/buildinfo.h"
  "static char *buildinfo = \"Configured with CMake ${CMAKE_VERSION} for native Windows\";\n")

set(GRADS_PREFIX "$ENV{LIBRARY_PREFIX}")
if(NOT GRADS_PREFIX)
  set(GRADS_PREFIX "$ENV{PREFIX}")
endif()

set(GRADS_INCLUDE_HINTS
  "${GRADS_PREFIX}/include"
  "${GRADS_PREFIX}/include/cairo"
  "${GRADS_PREFIX}/include/freetype2"
  "${GRADS_PREFIX}/include/geotiff"
  "${GRADS_PREFIX}/include/hdf4"
  "${GRADS_PREFIX}/include/hdf5"
  "${GRADS_PREFIX}/include/libxml2")
set(GRADS_LIBRARY_HINTS "${GRADS_PREFIX}/lib" "${GRADS_PREFIX}/bin")

function(grads_find_library variable)
  find_library(${variable} NAMES ${ARGN} HINTS ${GRADS_LIBRARY_HINTS} REQUIRED)
  set(${variable} "${${variable}}" PARENT_SCOPE)
endfunction()

find_path(GRADS_NETCDF_INCLUDE netcdf.h HINTS ${GRADS_INCLUDE_HINTS} REQUIRED)
find_path(GRADS_HDF4_INCLUDE mfhdf.h HINTS ${GRADS_INCLUDE_HINTS} PATH_SUFFIXES hdf4 REQUIRED)
find_path(GRADS_HDF5_INCLUDE hdf5.h HINTS ${GRADS_INCLUDE_HINTS} PATH_SUFFIXES hdf5 REQUIRED)
find_path(GRADS_UDUNITS_INCLUDE udunits2.h HINTS ${GRADS_INCLUDE_HINTS} REQUIRED)
find_path(GRADS_GRIB2_INCLUDE grib2.h HINTS ${GRADS_INCLUDE_HINTS} REQUIRED)
find_path(GRADS_GEOTIFF_INCLUDE geotiff.h HINTS ${GRADS_INCLUDE_HINTS} PATH_SUFFIXES geotiff REQUIRED)
find_path(GRADS_SHAPE_INCLUDE shapefil.h HINTS ${GRADS_INCLUDE_HINTS} REQUIRED)
find_path(GRADS_CAIRO_INCLUDE cairo.h HINTS ${GRADS_INCLUDE_HINTS} PATH_SUFFIXES cairo REQUIRED)
find_path(GRADS_GD_INCLUDE gd.h HINTS ${GRADS_INCLUDE_HINTS} REQUIRED)
find_path(GRADS_X11_INCLUDE X11/Xlib.h HINTS ${GRADS_INCLUDE_HINTS} REQUIRED)

grads_find_library(GRADS_NETCDF_LIBRARY netcdf)
grads_find_library(GRADS_HDF4_MFHDF_LIBRARY mfhdf)
grads_find_library(GRADS_HDF4_DF_LIBRARY hdf libhdf df)
grads_find_library(GRADS_HDF5_LIBRARY hdf5)
grads_find_library(GRADS_HDF5_HL_LIBRARY hdf5_hl)
grads_find_library(GRADS_UDUNITS_LIBRARY udunits2)
if(NOT GRADS_G2C_LIBRARY)
  grads_find_library(GRADS_G2C_LIBRARY g2c)
endif()
grads_find_library(GRADS_JASPER_LIBRARY jasper)
grads_find_library(GRADS_PNG_LIBRARY png16 png libpng16)
grads_find_library(GRADS_ZLIB_LIBRARY zlib z)
grads_find_library(GRADS_JPEG_LIBRARY jpeg)
grads_find_library(GRADS_GEOTIFF_LIBRARY geotiff geotiff_i)
grads_find_library(GRADS_TIFF_LIBRARY tiff)
grads_find_library(GRADS_SHAPE_LIBRARY shapefile shp)
grads_find_library(GRADS_CAIRO_LIBRARY cairo)
grads_find_library(GRADS_GD_LIBRARY gd)
grads_find_library(GRADS_X11_LIBRARY X11 libX11)
grads_find_library(GRADS_XEXT_LIBRARY Xext libXext)

set(GRADS_INCLUDE_DIRS
  src "${GRADS_GENERATED_DIR}" src/wincompat
  ${GRADS_NETCDF_INCLUDE} ${GRADS_HDF4_INCLUDE} ${GRADS_HDF5_INCLUDE}
  ${GRADS_UDUNITS_INCLUDE} ${GRADS_GRIB2_INCLUDE} ${GRADS_GEOTIFF_INCLUDE}
  ${GRADS_SHAPE_INCLUDE} ${GRADS_CAIRO_INCLUDE} ${GRADS_GD_INCLUDE}
  ${GRADS_X11_INCLUDE} ${GRADS_INCLUDE_HINTS})

set(GRADS_DEPENDENCY_LIBRARIES
  ${GRADS_NETCDF_LIBRARY} ${GRADS_HDF4_MFHDF_LIBRARY} ${GRADS_HDF4_DF_LIBRARY}
  ${GRADS_HDF5_HL_LIBRARY} ${GRADS_HDF5_LIBRARY} ${GRADS_UDUNITS_LIBRARY}
  ${GRADS_G2C_LIBRARY} ${GRADS_JASPER_LIBRARY} ${GRADS_PNG_LIBRARY}
  ${GRADS_ZLIB_LIBRARY} ${GRADS_JPEG_LIBRARY} ${GRADS_GEOTIFF_LIBRARY}
  ${GRADS_TIFF_LIBRARY} ${GRADS_SHAPE_LIBRARY} ${GRADS_CAIRO_LIBRARY}
  ${GRADS_GD_LIBRARY} ${GRADS_X11_LIBRARY} ${GRADS_XEXT_LIBRARY})

set(GRADS_ENGINE_SOURCES
  src/grads.c src/ga_dynload.c src/ga_readline_win.c src/gxsubs.c src/gxmeta.c
  src/gxchpl.c src/gxcntr.c src/gxstrm.c src/gxdb.c src/gxwmap.c src/gxshad.c
  src/gxshad2.c src/gaexpr.c src/gafunc.c src/gautil.c src/gagx.c src/gscrpt.c
  src/gamach.c src/gatxt.c src/galloc.c src/gaddes.c src/gacfg.c src/gaio.c
  src/gauser.c src/gasdf.c src/bufrstn.c src/gabufr.c src/gabufrtbl.c)

function(grads_configure_target target)
  target_include_directories(${target} PRIVATE ${GRADS_INCLUDE_DIRS})
  target_compile_definitions(${target} PRIVATE HAVE_CONFIG_H _CRT_SECURE_NO_WARNINGS)
  if(MSVC)
    target_compile_options(${target} PRIVATE "/FI${CMAKE_CURRENT_SOURCE_DIR}/src/ga_wincompat.h")
  else()
    target_compile_options(${target} PRIVATE -include "${CMAKE_CURRENT_SOURCE_DIR}/src/ga_wincompat.h")
  endif()
endfunction()

add_library(gradspy SHARED ${GRADS_ENGINE_SOURCES})
target_compile_definitions(gradspy PRIVATE SHRDOBJ)
target_link_libraries(gradspy PRIVATE ${GRADS_DEPENDENCY_LIBRARIES})
grads_configure_target(gradspy)
set_target_properties(gradspy PROPERTIES PREFIX "" OUTPUT_NAME "gradspy")

add_executable(grads src/grads_main.c)
target_link_libraries(grads PRIVATE gradspy)
grads_configure_target(grads)

add_library(gxdummy SHARED src/gxdummy.c)
add_library(gxdX11 SHARED src/gxX11.c)
add_library(gxpGD SHARED src/gxprintGD.c src/gxGD.c)
add_library(gxpCairo SHARED src/gxprint.c src/gxC.c)
add_library(gxdCairo SHARED src/gxX.c src/gxC.c)

foreach(plugin gxdummy gxdX11 gxpGD gxpCairo gxdCairo)
  grads_configure_target(${plugin})
  target_link_libraries(${plugin} PRIVATE gradspy ${GRADS_DEPENDENCY_LIBRARIES})
  set_target_properties(${plugin} PROPERTIES PREFIX "lib")
endforeach()

add_library(grads_utility_common OBJECT src/gautil.c src/gatxt.c)
target_compile_definitions(grads_utility_common PRIVATE STNDALN)
grads_configure_target(grads_utility_common)

add_library(grads_descriptor OBJECT src/gaddes.c)
target_compile_definitions(grads_descriptor PRIVATE STNDALN)
grads_configure_target(grads_descriptor)

function(grads_add_utility target)
  add_executable(${target} ${ARGN} $<TARGET_OBJECTS:grads_utility_common>)
  grads_configure_target(${target})
  target_link_libraries(${target} PRIVATE ${GRADS_DEPENDENCY_LIBRARIES})
endfunction()

grads_add_utility(stnmap src/stnmap.c src/gamach.c src/galloc.c $<TARGET_OBJECTS:grads_descriptor>)
grads_add_utility(gribmap src/gribmap.c src/gagmap.c src/gamach.c src/galloc.c $<TARGET_OBJECTS:grads_descriptor>)
grads_add_utility(gribscan src/gribscan.c src/gamach.c src/galloc.c)
grads_add_utility(grib2scan src/grib2scan.c src/gamach.c src/galloc.c)
add_executable(bufrscan src/bufrscan.c src/gabufr.c src/gabufrtbl.c src/gamach.c)
grads_configure_target(bufrscan)
target_link_libraries(bufrscan PRIVATE ${GRADS_DEPENDENCY_LIBRARIES})

install(TARGETS grads gradspy gxdummy gxdX11 gxpGD gxpCairo gxdCairo
  stnmap gribmap gribscan grib2scan bufrscan
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_BINDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
