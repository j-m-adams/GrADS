dset ^wrf_sample.nc
dtype netcdf
undef -888
TITLE WRF Output Grid: Time, bottom_top, south_north, west_east
pdef 249 249 lcc 26.9628 -125.898 1 1 36.2999 36.2999 -116.0 8000 8000
xdef 270 linear -129 0.1
ydef 200 linear   26 0.1
zdef  25 linear 1 1
tdef   2 linear 11jun2002 3hr
vars 4
P=>p      25  t,z,y,x  Pressure
T=>t      25  t,z,y,x  perturbation potential temperature (theta-t0)
HGT=>hgt   0  t,y,x    terrain height
T2=>t2     0  t,y,x    temperature at 2m
endvars
