* This is a GrADS descriptor file for NCEP GRIB2 file downloaded from:
*   ftp://ftpprd.ncep.noaa.gov/pub/data/nccf/com/gens/prod/gefs.yyyymmdd/hh/pgrb2alr/
*
* N.B. In this example descriptor file, the NCEP grib2 file name is changed 
* from:
*    geENS.tHHz.pgrb2af* 
*        (the filename on the NCEP FTP server)
* to:
*    gfsens.ENS.yyyymmddhh.fff.2p5.grib2
*        (a filename that includes initialization date, forecast hour, and ensemble name)
*
* where:
*     yyyy is the initialization year
*       mm is the initialization month
*       dd is the initialization day
*       HH is the initialization hour
*      ENS is the ensemble name 
*
dset /your_path_name_here/gfsens.%e.%iy4%im2%id2%ih2.%f3.2p5.grib2
dtype grib2
title NCEP 5D Ensemble Forecast in GRIB2 on a 2.5-degree grid
index ^gfsens.map
options template pascals
undef -9.99e33
xdef 144 linear  0.0 2.5
ydef  73 linear -90.0 2.5
zdef   7 levels 100000 92500 85000 70000 50000 25000 20000
tdef  65 linear 12z24jan08 6hr
edef 21
*name length initialtime <type,pert or deriv>
c00  65  12z24jan08  1,0
p01  65  12z24jan08  3,1
p02  65  12z24jan08  3,2
p03  65  12z24jan08  3,3
p04  65  12z24jan08  3,4
p05  65  12z24jan08  3,5
p06  65  12z24jan08  3,6
p07  65  12z24jan08  3,7
p08  65  12z24jan08  3,8
p09  65  12z24jan08  3,9
p10  65  12z24jan08  3,10
p11  65  12z24jan08  3,11
p12  65  12z24jan08  3,12
p13  65  12z24jan08  3,13
p14  65  12z24jan08  3,14
p15  65  12z24jan08  3,15
p16  65  12z24jan08  3,16
p17  65  12z24jan08  3,17
p18  65  12z24jan08  3,18
p19  65  12z24jan08  3,19
p20  65  12z24jan08  3,20
endedef
vars 22
hgt    7,100         0,3,5      Geopotential Height [gpm]
tmp    7,100         0,0,0      Temperature [K]
rh     7,100         0,1,1      Relative Humidity [%]
u      7,100         0,2,2      U-Component of Wind [m/s]
v      7,100         0,2,3      V-Component of Wind [m/s]
* these surface vars valid at initial time only 
zs     0,1,0         0,3,5      Surface Geopotential Height [gmp]
* these surface vars valid at all times
ps     0,1,0         0,3,0      Surface Pressure [Pa]
pwat   0,200,0       0,1,3      Precipitable Water [kg/m^2]
cape   0,108,18000,0 0,7,6      CAPE, 180-0 mb above ground [J/kg]
slp    0,101,0       0,3,1      Mean Sea Level Pressure [Pa]
t2     0,103,2       0,0,0      2-meter Temperature [K]
rh2m   0,103,2       0,1,1      2-meter Relative Humidity [%]
u10    0,103,10      0,2,2      10-meter U-Component of Wind [m/s]	
v10    0,103,10      0,2,3      10-meter V-Component of Wind [m/s]
* these surface vars valid at forecast times only
t2max  0,103,2       0,0,4,2    2-meter Maximum Temperature [K]	
t2min  0,103,2       0,0,5,3    2-meter Minimum Temperature [K]	
tc     0,200,0       0,6,1,0    Total Cloud Cover [%]
p      0,1,0         0,1,8,1    Accumulated Precipitation [kg/m^2] 
crain  0,1,0         0,1,192,0  Categorical Rain (yes=1; no=0)
cfrzr  0,1,0         0,1,193,0  Categorical Freezing Rain (yes=1; no=0)
cicep  0,1,0         0,1,194,0  Categorical Ice Pellets (yes=1; no=0)
csnow  0,1,0         0,1,195,0  Categorical Snow (yes=1; no=0)
endvars
