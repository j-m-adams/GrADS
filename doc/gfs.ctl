* This is a GrADS descriptor file for NCEP GRIB2 file downloaded from:
*   ftp://tgftp.nws.noaa.gov/SL.us008001/ST.opnl/MT.gfs_CY.hh/RD.yyyymmdd/PT.grid_DF.gr2/
*
* N.B. In this example descriptor file, the NCEP grib2 file name is changed 
* from:
*   fh.0fff_tl.press_gr.0p5deg 
*      (the filename on the NCEP FTP server)
* to:
*   gfs.yyyymmddhh.fff.grib2 
*      (a filename that includes initialization date and forecast hour)
*
* where:
*     yyyy is the initialization year
*       mm is the initialization month
*       dd is the initialization day
*       hh is the initialization hour
*      fff is the forecast hour
*
dset /your_path_name_here/gfs.%iy4%im2%id2%ih2.%f3.grib2
title GFS Forecast in GRIB2; 3-hourly out to 180 hours on a 0.5-degree grid
dtype grib2
index ^gfs.map
options template pascals
undef -9.99e33
xdef 720 linear  0 0.5
ydef 361 linear -90 0.5
zdef 26 levels 100000 97500 95000 92500 90000 85000 80000 
    75000 70000 65000 60000 55000 50000 45000 40000 35000 
    30000 25000 20000 15000 10000 7000 5000 3000 2000 1000 
tdef 61 linear 12z24jan08 3hr
vars 144
z           26,100           0,3,5      Geopotential Height [gpm]
t           26,100           0,0,0      Temperature [K]
u           26,100           0,2,2      U-Component of Wind [m/s]
v           26,100           0,2,3      V-Component of Wind [m/s]
vort        26,100           0,2,10     Absolute Vorticity [/s]
vv          21,100           0,2,8      Vertical Velocity (Pressure) [Pa/s]
rh          21,100           0,1,1      Relative Humidity [%]
clwmr       21,100           0,1,22     Cloud Water Mixing Ratio [kg/kg]
ps          0,1,0            0,3,0      Surface Pressure [Pa]
ts          0,1,0            0,0,0      Surface Temperature [K]
t2m         0,103,2          0,0,0      2m Temperature [K]
t2min       0,103,2          0,0,4      2m Minimum Temperature [K]
t2max       0,103,2          0,0,5      2m Maximum Temperature [K]
q2          0,103,2          0,1,0      2m Specific Humidity [kg/kg]
p           0,1,0            0,1,8,1    Total Accumulated Precipitation [kg/m2]
pc          0,1,0            0,1,10,1   Convective Precipitation [kg/m2]
crain       0,1,0            0,1,192,0  Categorical Rain (yes=1; no=0)
cfrzr       0,1,0            0,1,193,0  Categorical Freezing Rain (yes=1; no=0)
cicep       0,1,0            0,1,194,0  Categorical Ice Pellets (yes=1; no=0)
csnow       0,1,0            0,1,195,0  Categorical Snow (yes=1; no=0)
pwat        0,200,0          0,1,3      Precipitable Water (NCEP level 200) [kg/m^2]
rhum        0,200,0          0,1,1      Relative Humidity (NCEP level 200) [%]
o3mr100     0,100,10000      0,14,192   100mb Ozone Mixing Ratio [kg/kg]
o3mr70      0,100,7000       0,14,192   70mb Ozone Mixing Ratio [kg/kg]
o3mr50      0,100,5000       0,14,192   50mb Ozone Mixing Ratio [kg/kg]
o3mr30      0,100,3000       0,14,192   30mb Ozone Mixing Ratio [kg/kg]
o3mr20      0,100,2000       0,14,192   20mb Ozone Mixing Ratio [kg/kg]
o3mr10      0,100,1000       0,14,192   10mb Ozone Mixing Ratio [kg/kg]
wavh500     0,100,50000      0,3,193    5-Wave Geopotential Height [gpm]
wava500     0,100,50000      0,3,197    5-Wave Geopotential Height Anomaly [gpm]
soilw1      0,106,0,0.1      2,0,192    Volumetric Soil Moisture,0.0-0.1m below surface [fraction]
soilw2      0,106,0.1,0.4    2,0,192    Volumetric Soil Moisture,0.1-0.4m below surface [fraction]
soilw3      0,106,0.4,1      2,0,192    Volumetric Soil Moisture,0.4-1.0m below surface [fraction]
soilw4      0,106,1,2        2,0,192    Volumetric Soil Moisture,1.0-2.0m below surface [fraction]
soilt1      0,106,0,0.1      0,0,0      Soil Temperature,0.0-0.1m below surface [K]   
soilt2      0,106,0.1,0.4    0,0,0      Soil Temperature,0.1-0.4m below surface [K] 
soilt3      0,106,0.4,1      0,0,0      Soil Temperature,0.4-1.0m below surface [K]   
soilt4      0,106,1,2        0,0,0      Soil Temperature,1.0-2.0m below surface [K]     
tb          0,108,3000,0     0,0,0      Bottom 30mb Temperature [K]
rhb         0,108,3000,0     0,1,1      Bottom 30mb Relative Humidity [%]
qb          0,108,3000,0     0,1,0      Bottom 30mb Specific Humidity [kg/kg]
ub          0,108,3000,0     0,2,2      Bottom 30mb U Winds [m/s]
vb          0,108,3000,0     0,2,3      Bottom 30mb V Winds [m/s] 
t6000       0,102,1829       0,0,0      Temperature at 6000ft AMSL [K]
t9000       0,102,2743       0,0,0      Temperature at 9000ft AMSL [K]
t12000      0,102,3658       0,0,0      Temperature at 12000ft AMSL [K]
u6000       0,102,1829       0,2,2      U Winds at 6000ft AMSL [m/s]
u9000       0,102,2743       0,2,3      U Winds at 9000ft AMSL [m/s]
u12000      0,102,3658       0,2,2      U Winds at 12000ft AMSL [m/s]
v6000       0,102,1829       0,2,3      V Winds at 6000ft AMSL [m/s]
v9000       0,102,2743       0,2,2      V Winds at 9000ft AMSL [m/s]
v12000      0,102,3658       0,2,3      V Winds at 12000ft AMSL [m/s]
ztrop       0,7,0            0,3,5      Tropopause Geopotential Height [gpm]
ttrop       0,7,0            0,0,0      Tropopause Temperature [K]
ptrop       0,7,0            0,3,0      Tropopause Pressure [Pa]
utrop       0,7,0            0,2,2      Tropopause U Winds [m/s]
vtrop       0,7,0            0,2,3      Tropopause V Winds [m/s]
strop       0,7,0            0,2,192    Tropopause Vertical Speed Shear [/s]
rhl1        0,104,0.33,1     0,1,1      Relative Humidity, Sigma 0.33 to 1 [%]
rhl2        0,104,0.44,1     0,1,1      Relative Humidity, Sigma 0.44 to 1 [%]
rhl3        0,104,0.72,0.94  0,1,1      Relative Humidity, Sigma 0.72 to 0.94 [%]
rhl4        0,104,0.44,0.72  0,1,1      Relative Humidity, Sigma 0.44 to 0.72 [%]
ptls        0,104,0.995      0,0,2      Sigma 0.995 Potential Temperature [K]
tls         0,104,0.995      0,0,0      Sigma 0.995 Temperature [K]
vvls        0,104,0.995      0,2,8      Sigma 0.995 Vertical Velocity [Pa/s]
rhls        0,104,0.995      0,1,1      Sigma 0.995 Relative Humidity [%]
uls         0,104,0.995      0,2,2      Sigma 0.995 U Winds [m/s]
vls         0,104,0.995      0,2,3      Sigma 0.995 V Winds [m/s]
sli         0,1,0            0,7,192    Surface Lifted Index [K]
capes       0,1,0            0,7,6      Convective Available Potential Energy (Surface) [J/kg]
cins        0,1,0            0,7,7      Convective Inhibition (Surface) [J/kg]
li          0,1,0            0,7,193    Best (4 Layer) Lifted Index [K]
cape        0,108,18000,0    0,7,6      Convective Available Potential Energy (Lowest 180mb) [J/kg]
cin         0,108,18000,0    0,7,7      Convective Inhibition (Lowest 180mb)  [J/kg]
zwmx        0,6,0            0,3,5      Max Wind Level Geopotential Height [gpm]
twmx        0,6,0            0,0,0      Max Wind Level Temperature [K]
pwmx        0,6,0            0,3,0      Max Wind Level Pressure [Pa]
uwmx        0,6,0            0,2,2      Max Wind Level U Winds [m/s]
vwmx        0,6,0            0,2,3      Max Wind Level V Winds [m/s]
zs          0,1,0            0,3,5      Surface Geopotential Height [gpm]
slp         0,101,0          0,3,1      Sea Level Pressure [Pa]
dlwrfs      0,1,0            0,5,192,0  Surface Downward Long Wave Rad. Flux [W/m^2]
ulwrfs      0,1,0            0,5,193,0  Surface Upward Long Wave Rad. Flux [W/m^2]
ulwrft      0,8,0            0,5,193,0  Top of Atmosphere Upward Long Wave Rad. Flux [W/m^2]
uswrft      0,8,0            0,4,193,0  Top of Atmosphere Upward Short Wave Rad. Flux [W/m^2]
uswrfs      0,1,0            0,4,193,0  Surface Upward Short Wave Rad. Flux [W/m^2]
dswrfs      0,1,0            0,4,192,0  Surface Downward Short Wave Rad. Flux [W/m^2]
z0c         0,4,0            0,3,5      Geopotential Height at 0C Isotherm [gpm]
rh0c        0,4,0            0,1,1      Relative Humidity  at 0C Isotherm [%]
shtfls      0,1,0            0,0,11,0   Surface Sensible Heat Net Flux [W/m^2]
lhtfls      0,1,0            0,0,10,0   Surface Latent Heat Net Flux [W/m^2]
weasd       0,1,0            0,1,13     Accumulated Snow Depth (water equivalent) [kg/m^2]
prate       0,1,0            0,1,7,0    Precipitation Rate [kg/m^2 s^1]
cprate      0,1,0            0,1,196,0  Convective Precipitation Rate [kg/m^2 s^1]
gflux       0,1,0            2,0,193,0  Ground Heat Flux [W/m^2]
land        0,1,0            2,0,0      Land Cover (1=land, 0=sea)
icec        0,1,0            10,2,0     Ice Cover [Proportion]
runoff      0,1,0            2,0,5,1    Surface Water Runoff [kg/m^2]
pevpr       0,1,0            0,1,200    Surface Potential Evaporation Rate [W/m^2]     
hpbl        0,1,0            0,3,196    Planetary Boundary Layer Height [m]
albedo      0,1,0            0,19,1,0   Surface Albedo [%]
rh2m        0,103,2          0,1,1      2m Relative Humidity [%]
uflx        0,1,0            0,2,17,0   Surface U-Momentum Flux [N/m^2]
vflx        0,1,0            0,2,18,0   Surface v-Momentum Flux [N/m^2]
u10m        0,103,10         0,2,2      10m U Winds [m/s]
v10m        0,103,10         0,2,3      10m V Winds [m/s]
ugwd        0,1,0            0,3,194,0  Zonal Flux of Gravity Wave Stress [N/m^2]
vgwd        0,1,0            0,3,195,0  Meridional Flux of Gravity Wave Stress [N/m^2]
gpa1000     0,100,100000     0,3,9      1000mb Geopotential Height Anomaly [gpm]
gpa500      0,100,50000      0,3,9      500mb Geopotential Height Anomaly [gpm]
ozone200    0,200,0          0,14,0     NCEP level type 200 Total Ozone [Dobson]
cwat200     0,200,0          0,6,6      NCEP level type 200 Cloud Water [kg/m^2]
cwork200    0,200,0          0,6,193,0  NCEP level type 200 Cloud Work Function [J/kg]
hgt204      0,204,0          0,3,5      NCEP level type 204 Geopotential Height [gpm]
rh204       0,204,0          0,1,1      NCEP level type 204 Relative Humidity [%]
pres212     0,212,0          0,3,0,0    NCEP level type 212 Pressure [Pa]
pres213     0,213,0          0,3,0,0    NCEP level type 213 Pressure [Pa]
pres222     0,222,0          0,3,0,0    NCEP level type 222 Pressure [Pa]
pres223     0,223,0          0,3,0,0    NCEP level type 223 Pressure [Pa]
pres232     0,232,0          0,3,0,0    NCEP level type 232 Pressure [Pa]
pres233     0,233,0          0,3,0,0    NCEP level type 233 Pressure [Pa]
pres242     0,242,0          0,3,0      NCEP level type 242 Pressure [Pa]
pres243     0,243,0          0,3,0      NCEP level type 243 Pressure [Pa]
tcc200      0,200,0          0,6,1,0    NCEP level type 200 Total Cloud Cover [%]
tcc211      0,211,0          0,6,1,0    NCEP level type 211 Total Cloud Cover [%]
tcc214      0,214,0          0,6,1,0    NCEP level type 214 Total Cloud Cover [%]
tcc224      0,224,0          0,6,1,0    NCEP level type 224 Total Cloud Cover [%]
tcc234      0,234,0          0,6,1,0    NCEP level type 234 Total Cloud Cover [%]
tcc244      0,244,0          0,6,1      NCEP level type 244 Total Cloud Cover [%]
t213        0,213,0          0,0,0,0    NCEP level type 213 Temperature [K]
t223        0,223,0          0,0,0,0    NCEP level type 223 Temperature [K]
t233        0,233,0          0,0,0,0    NCEP level type 233 Temperature [K]
hgt2pv      0,109,2e-6       0,3,5      2PotVortSfc Geopotential Height [gpm]
hgtneg2pv   0,109,-2e-6      0,3,5      neg2PotVortSfc Geopotential Height [gpm]
pres2pv     0,109,2e-6       0,3,0      2PotVortSfc Pressure [Pa]
presneg2pv  0,109,-2e-6      0,3,0      neg2PotVortSfc Pressure [Pa]
t2pv        0,109,2e-6       0,0,0      2PotVortSfc Temperature [K]
tneg2pv     0,109,-2e-6      0,0,0      neg2PotVortSfc Temperature [K]
u2pv        0,109,2e-6       0,2,2      2PotVortSfc U-Component of Wind [m/s]
uneg2pv     0,109,-2e-6      0,2,2      neg2PotVortSfc U-Component of Wind [m/s]
v2pv        0,109,2e-6       0,2,3      2PotVortSfc V-Component of Wind [m/s]
vneg2pv     0,109,-2e-6      0,2,3      neg2PotVortSfc V-Component of Wind [m/s]
vss2pv      0,109,2e-6       0,2,192    2PotVortSfc Vertical speed sheer [1/s]
vssneg2pv   0,109,-2e-6      0,2,192    neg2PotVortSfc Vertical speed sheer [1/s]
endvars
