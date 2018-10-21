dnl acinclude.m4: 
dnl
dnl  This file contains custom M4 macro definitions for the GrADS build system.
dnl  It is merged with automake-related macros in "aclocal.m4" by the "aclocal"
dnl  program. The merged "aclocal.m4" file is in turn used by the "autoconf" 
dnl  program to generate the "configure" script. 
dnl


dnl GA_SET_SUPPLIBS : Find a supplibs directory. 
dnl		      If SUPPLIBS environment variable is not set, then 
dnl  		      search for "supplibs" in the paths given in the args
dnl args: paths-to-search (eg. [. ..])
AC_DEFUN([GA_SET_SUPPLIBS],
[ 
  AC_MSG_CHECKING([for supplibs directory])
  if test -n "${SUPPLIBS}" ; then
    # Use present supplib name unmodified, assume it is absolute path
    AC_MSG_RESULT([${SUPPLIBS}])
    # This is the "official" variable name for use by other macros
    ga_supplib_dir="${SUPPLIBS}"
  else
    # Look for "supplibs" directory in ${top_builddir}
    for ga_supplib_prefix in $1 ; do 
      SUPPLIBS="${ga_supplib_prefix}/supplibs"
      if test -d "${SUPPLIBS}" ; then
        AC_MSG_RESULT([${SUPPLIBS}])
        break
      fi 
    done
    if test ! -d "${SUPPLIBS}" ; then
      AC_MSG_RESULT([not found])
      SUPPLIBS=""
    fi
    # This is the "official" variable name for use by other macros
    ga_supplib_dir=$SUPPLIBS
    # Add prefix so that Makefiles in subdirectories can find it
    SUPPLIBS='$(top_builddir)/'"$SUPPLIBS"
  fi
  AC_SUBST(SUPPLIBS)
])

dnl GA_SET_FLAGS : Sets the compile and link paths to supplibs, plus any extra
dnl 		   compiler or linker flags given, and saves original settings
dnl 		   for restoration by GA_UNSET_FLAGS
dnl  args: 	   extra_supplib_inc_names, extra-CPP-flags, extra-LD-flags, 
dnl                   extra-LIB-flags
AC_DEFUN([GA_SET_FLAGS],
[
  # Use to make temporary changes to -I and -L paths 
  # Just for use during tests, because configure and make may run 
  # from different directories. 
  ga_saved_cppflags=$CPPFLAGS
  ga_saved_ldflags=$LDFLAGS
  ga_saved_libs=$LIBS
  CPPFLAGS="-I${ga_supplib_dir}/include"
  m4_if([$1], [], [:], [
    for ga_inc_name in $1 ; do
      CPPFLAGS="$CPPFLAGS -I${ga_supplib_dir}/include/${ga_inc_name}"
    done
  ])
  CPPFLAGS="$CPPFLAGS $2"
  LDFLAGS="-L${ga_supplib_dir}/lib $3"
  LIBS="$LIBS $4"
])

dnl GA_UNSET_FLAGS : Undoes changes to compiler and linker flags made by GA_SET_FLAGS.
dnl  args:	     none
AC_DEFUN([GA_UNSET_FLAGS],
[
  # Use to undo temporary changes to -I and -L paths 
  CPPFLAGS=$ga_saved_cppflags
  LDFLAGS=$ga_saved_ldflags
  LIBS=$ga_saved_libs
])

dnl GA_SET_LIB_VAR : Puts necessary linker options to link with libraries given into
dnl                  a shell variable. They will have the form 'supplib_dir/libname.a'.
dnl   args:	   : shell-variable-name, list-of-libraries (e.g. [readline termcap])
AC_DEFUN([GA_SET_LIB_VAR],
[
  ga_lib_prefix='$(supp_lib_dir)/lib'
  ga_lib_suffix='.a'
  for ga_lib_name in $2 ; do
      $1="$$1 ${ga_lib_prefix}${ga_lib_name}${ga_lib_suffix}"
  done  
])

dnl GA_SET_DYNLIB_VAR : Puts necessary linker options to link dynamically with libraries given into
dnl                  a shell variable. They will have the form '-lname'.
dnl   args:	   : shell-variable-name, list-of-libraries (e.g. [cairo_libs cairo])
AC_DEFUN([GA_SET_DYNLIB_VAR],
[
  ga_lib_prefix='-l'
  for ga_lib_name in $2 ; do
      $1="$$1 ${ga_lib_prefix}${ga_lib_name}"
  done  
])

dnl GA_SET_INCLUDE_VAR : Puts necessary options to compile with include directories 
dnl                      given into a shell variable. 
dnl   args:	   : shell-variable-name, list-of-directories 
AC_DEFUN([GA_SET_INCLUDE_VAR],
[
  ga_include_prefix='-I$(supp_include_dir)'

  for ga_include_name in $2 ; do
      $1="$$1 ${ga_include_prefix}/${ga_include_name}"
  done  
])

