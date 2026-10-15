---
title: set vpage
---

# **set vpage**

`set vpage `*`xmin xmax ymin ymax`*

This command defines a "virtual page" that fits within the specified limits of the real page. All graphics output will be drawn into this "virtual page" until another `set vpage` command is entered. A `clear` command clears the physical page (and any virtual pages drawn on it).

When GrADS is first started, it prompts for landscape or portrait mode. This defines the size of the real page (11x8.5 or 8.5x11), and the dimensions for the virtual page must fit within this real page.

The `set vpage` command will define virtual page limits in terms of inches (virtual page inches), which are the coordinates that will be used in the various commands that require inches to be used. The new page limits are printed when the `set vpage` command completes.

To return to the default state where the real page equals the virtual page, enter:

`set vpage off `

## Usage Note

### Examples
