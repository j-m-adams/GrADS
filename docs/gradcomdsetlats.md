---
title: set lats
---

# **set lats**

`set lats `*`arguments`*

Valid *`arguments`* are:

```text

parmtab    
filename
 
convention [ grads_grib | grib_only | coards ]
calendar   [ standard | noleap | clim | climleap ]
frequency  [ yearly | monthly | monthly_table_comp | weekly | daily | hourly | forecast_hourlyfixed] 
delatat    
n
  
fhour      
n

model      
model_name

center     
center_name

create     
filename

comment    
string

gridtype   [ linear | gaussian | generic ]
vertdim    DIMNAME 
val1 val2 ... valN
 
var        VARNAME [ average | accum | instant ] LEVEL_ID
timeoption [ grid | dim_env ]
write      VAR_ID 
level
 
close 

```

## Usage Notes

### Examples

```text

set lats parmtab lats.ncep.MRFtable
set lats convention coards
set lats calendar standard
set lats frequency hourly
set lats deltat 6
set lats fhour 120
set lats model MRF
set lats center NCEP
set lats create MRF.EXP1
set lats comment "Latest MRF forecast with convection update"
set lats gridtype gaussian
set lats vertdim plev 1000 850 500 200
set lats var u instant 1
set lats v timeoption dim_env (use the GrADS dimension environment)
set lats write 1 500 (return from t lats var)
set lats close

```
