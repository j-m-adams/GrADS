---
title: set imprun
---

# **set imprun**

`set imprun `*`script-name`*

This command sets up automatic execution of a Grads script before every <a href="gradcomddisplay.html">`display`</a> command.

## Examples

This script is typically used to set an option that by default gets reset after each <a href="gradcomddisplay.html">`display`</a> command, for example:

<a href="gradcomdsetgrads.html">`set grads`</a>` off`

### Usage Notes

You can issue any GrADS command from this script, but the interactions are not always clear. For example, if you issue a <a href="gradcomddisplay.html">`display`</a> command from this script, you could enter an infinite recursion loop.

The argument to the script is the expression from the <a href="gradcomddisplay.html">`display`</a> command.
