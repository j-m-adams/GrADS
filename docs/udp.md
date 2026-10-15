---
title: User Defined Plug-in Functions (UDPs)
---

# User Defined Plug-in Functions (UDPs)

<a href="#overview">Overview</a>\
<a href="#how">How to set up and use User Defined Plug-in Functions</a>\
<a href="#compile">How to compile User Defined Plug-in Functions</a>\
<a href="#table">The User Defined Plug-in Table</a>\
<a href="#gaudpt">The environment variable GAUDPT</a>\
<a href="#example1">Example: Add a number to a variable</a>\

------------------------------------------------------------------------


## Overview

User Defined <a href="plugins.html">Plug-in</a> functions were introduced in version 2.1.1.b0 and replace the legacy file-based user-defined-function interface, which was disabled when version 2.0 was introduced. The use of plug-in functions improves performance and flexibility for users who want to create customized functions. The function arguments and data grids are no longer passed between GrADS and the user's program through data files written out to disk. With plug-in functions, the user's code is dynamically loaded into GrADS when the function is invoked by the user, and the data is operated on while still sitting in memory. Please read the following documentation carefully.

(how)=

### How to set up and use User Defined Plug-in functions:

1.  Compile your plug-in functions as shared object files. (Scroll down for <a href="#compile">additional documention</a>.)
2.  Update a stand-alone text file called the User Defined Plug-in Table (<a href="udpt.html">UDPT</a>) that provides all the necessary information GrADS needs to know about plug-ins. (Scroll down for <a href="#table">additional documention.</a>)
3.  Set the environment variable GAUDPT to provide the full name (including the path) of the <a href="udpt.html">UDPT</a> file.
4.  When GrADS is first started up, it will parse the UDPT records so it knows all the plug-in function names and where to find them. The plug-ins will be loaded dynamically by GrADS only when the function is called by the user.\
5.  When a plug-in function is invoked in an <a href="expressions.html">expression</a>, GrADS passes all the function arguments as-is to the plug-in. The arguments to a plug-in generally contain one or more GrADS expressions plus any additional numbers or strings that might be needed. The plug-in should contain the necessary code to parse and check the arguments, evaluate the expressions(s), perform the calculations, print out any desired diagnostic information, and return the result back to GrADS.
6.  Note: User defined plug-ins do *NOT* have precedence over GrADS intrinsic functions, thus a UDP cannot be set up to replace a GrADS function. This behavior is different from the old user defined functions.

(compile)=

### How to compile User Defined Plug-in functions:

User Defined Plug-ins are compiled as dynamic libraries or shared object files and are loaded by GrADS using the dlopen(), dlsym(), and dlclose() functions. Compiling these dynamic object files is a two-step process that requires a slightly different syntax than what is normally used to compile a stand-alone executable. Consider an example plug-in program called addthis.c:

Compile the plug-in source code (`addthis.c`) and create the object file.

```console
  gcc -fPIC -Wall -g -c addthis.c 
```

    Note that this program requies the inclusion of `grads.h`, which is part of the GrADS source code. Use the environment variable CFLAGS to specify the directory where `grads.h` resides (e.g. \$HOME/grads/src/grads.h):

```console
  setenv CFLAGS -I$HOME/grads/src
  gcc -fPIC -Wall -g -c addthis.c $CFLAGS
```

If you get an error message that the compiler cannot find additional include files such as `shapefil.h`, then try adding the supplibs include directory where the file is located to the `CFLAGS` environment variable, or use the -D option to disable the USESHP macro:

```console
  gcc -fPIC -Wall -g -c addthis.c $CFLAGS -DUSESHP=0
```
2.  Once you have compiled the program and successfully created the object file `addthis.o`, you must create the dynamic library/shared object file that will be loaded by GrADS. More \`\`\`\` than one object file can be packaged in a dynamic library/shared object file. The synatx for this step is different for Linux systems and for Mac OS X:

    For Linux:

```console
  gcc -fPIC -g -shared -rdynamic addthis.o -o addthis.so 
```

    For Mac OS X:

```text
  libtool -dynamic -flat_namespace -undefined suppress addthis.o -o addthis.dylib 
```


### The User Defined Plug-in Table

The <a href="udpt.html">user-defined plug-in table (UDPT)</a> is a simple text file that contains information about a user defined plug-in function. A record in the UDPT is required before the plug-in funciton can be used in GrADS. Check the <a href="udpt.html">documentation</a> for more information about the proper syntax of UDPT records. An example record for our example plug-in `addthis.c` might look like this:

```text
   function addthis /home/username/grads/udp/addthis.so
```

(gaudpt)=

### The environment variable GAUDPT

GrADS will look for user defined plug-in function entries in two places: the file name pointed to by the <a href="gradcomdgrads.html#env">environment variable</a> GAUDPT, and a file named "udpt" in the directory named by the GADDIR environment variable. An example of setting the GAUDPT environment variable is:

```console
    setenv GAUDPT $HOME/grads/udpt
```


### Example: Add a number to a variable

**`addthis.c`** is a sample user defined plug-in function for use with GrADS that does a very basic task: it adds a number to all the non-missing values in a GrADS expression, which may be for gridded or station data. Additional information may be found in the comments of the source code.
