---
title: draw dropmenu
---

# **draw dropmenu**

`draw dropmenu `*`number xpos ypos width height text_list`*

Draws a drop menu with the following attributes:

- *`number      `* menu number, 0 - 64\
  *`xpos        `* x center of the menu base in page coordinates (inches)\
  *`ypos        `* y center of the menu base in page coordinates (inches)\
  *`width       `* width (x) of the menu base (inches)\
  *`height      `* height (y) of the menu base (inches)\
  *`text_list   `* the contents of the menu, seperated by vertical bars (\|)\

## Usage Notes

1.  The first item in the text list is the string to put in the 'base' of the dropmenu (the base being the part that always appears); the rest of the text are the menu items. Empty spaces are allowed in the strings.

2.  When the user clicks on the 'base', the rest of the menu appears.

3.  The menu colors are controlled by the <a href="gradcomdsetdropmenu.html">`set dropmenu`</a> command.

4.  Dropmenus can be nested or "cascading". The syntax for creating a "spawned" dropmenu is similar to that for the main dropmenu. First, any item in the *`text_list`* that will spawn a new dropmenu should have `">`*`num`*`>"` appended, where *`num`* will be the number assigned to the spawned dropmenu. This new dropmenu is then defined with the following syntax:

    `draw dropmenu `*`num`*` cascade `*`new_text_list`*

    There can be up to three levels of nested cascading dropmenus launched from the main dropmenu. The 2nd example below illustrates how to implement cascading dropmenus.

5.  The section of the User's Guide on <a href="script.html#widgets">widgets</a> and the <a href="gradcomdqpos.html">`q pos`</a> reference page have more information on using dropmenus.

### Examples

1.  Here is a script that illustrates how to use a simple dropmenu:
```text

'clear'
'reset events'
'set rgb 90 100 100 100'
'set rgb 91 150 150 150'
'set rgb 92 200 200 200'
'set dropmenu 1 91 90 92 0 91 92 90 1 91 90 92 92 90 6'
'draw dropmenu 1 1 8 1.5 0.5 Select a Variable | Wind | Temperature | Height | SLP '
noselect = 1
while (noselect)
  'q pos'
  menunum  = subwrd(result,7)
  menuitem = subwrd(result,8)
  if (menunum = 1)
    if menuitem = 1 ; newbase = 'Variable = Wind'   ; endif
    if menuitem = 2 ; newbase = 'Variable = Temp'   ; endif
    if menuitem = 3 ; newbase = 'Variable = Height' ; endif
    if menuitem = 4 ; newbase = 'Variable = SLP'    ; endif
    'draw dropmenu 1 1 8 1.5 0.5 'newbase' | Wind | Temperature | Height | SLP '
    noselect = 0
  endif
endwhile

```
2.  Here is a script that illustrates how to use cascading dropmenus:
```text

'clear'
'reset events'
'set rgb 90 100 100 100'
'set rgb 91 150 150 150'
'set rgb 92 200 200 200'
'set button 1 91 -1 -1 1 91 90 92 12'
'draw button 1 1 8 1 0.5 quit'
'set dropmenu 1 91 -1 -1 1 91 90 92 1 91 90 92 90 92 6'
'draw dropmenu 1 1.5 7.5 2 0.5  Menu Base | Space | Earth >05> | Sun | Moon'
'draw dropmenu 5 cascade Ocean | Land | Atmosphere >11> | Biosphere'
'draw dropmenu 11 cascade Snow | Rain | Mist | Tornado '

while (1)
  'q pos'
  say result
  ev = subwrd(result,6)
  if (ev!=3); break; endif;
endwhile

```
