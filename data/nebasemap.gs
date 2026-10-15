* Natural Earth land/ocean display mask for GrADS.
*
* Usage:
*   nebasemap L(and)/O(cean) <fill_color> <outline_color> <110m/50m/10m>
*
* Legacy resolution aliases L, M, and H select 110m, 50m, and 10m.

function main(args)

if (args='')
  say 'Usage: nebasemap L(and)/O(cean) <fill_color> <outline_color> <110m/50m/10m>'
  return
endif

type=subwrd(args,1)
fill=subwrd(args,2)
outline=subwrd(args,3)
res=subwrd(args,4)
if (fill='') ; fill=15 ; endif
if (outline='') ; outline=0 ; endif
if (res='') ; res='110m' ; endif

if (res='L' | res='l' | res='110') ; res='110m' ; endif
if (res='M' | res='m' | res='50')  ; res='50m'  ; endif
if (res='H' | res='h' | res='10')  ; res='10m'  ; endif
if (res!='110m' & res!='50m' & res!='10m')
  say 'nebasemap: resolution must be 110m, 50m, or 10m'
  return
endif

if (type='L' | type='l' | type='LAND' | type='land')
  kind='land'
else
  if (type='O' | type='o' | type='OCEAN' | type='ocean')
    kind='ocean'
  else
    say 'nebasemap: mask type must be L(and) or O(cean)'
    return
  endif
endif
file='ne'res'_'kind

* A plot must exist to establish scaling and projection.
'q gxinfo'
gxline=sublin(result,5)
xaxis=subwrd(gxline,3)
yaxis=subwrd(gxline,6)
if (xaxis='None' | yaxis='None')
  say 'nebasemap: display a variable before drawing the mask'
  return
endif

* Preserve the shapefile drawing settings.
'q shpopts'
shpline=sublin(result,2)
oldfill=subwrd(shpline,4)
shpline=sublin(result,3)
oldmark=subwrd(shpline,3)
shpline=sublin(result,4)
oldsize=subwrd(shpline,3)

* Limit the overlay to the existing plot area.
'q gxinfo'
gxline=sublin(result,3)
x1=subwrd(gxline,4)
x2=subwrd(gxline,6)
gxline=sublin(result,4)
y1=subwrd(gxline,4)
y2=subwrd(gxline,6)
'set clip 'x1' 'x2' 'y1' 'y2

* Use the fill color for polygon edges; draw the selected map outline last.
'set line 'fill' 1 1'
'set shpopts 'fill
'draw shp 'file
if (rc!=0)
  say 'nebasemap: unable to draw 'file
  'set shpopts 'oldfill' 'oldmark' 'oldsize
  return
endif

'set map 'outline
'draw map'
'set map auto'
'set line 1 1 6'
'draw rec 'x1' 'y1' 'x2' 'y2
'set shpopts 'oldfill' 'oldmark' 'oldsize
return
