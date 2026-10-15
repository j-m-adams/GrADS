/* Copyright (C) 1988-2022 by George Mason University. See file COPYRIGHT. */

#ifndef GA_DYNLOAD_H
#define GA_DYNLOAD_H

/*
 * Small cross-platform wrapper around the process dynamic loader.  Keeping
 * the platform API here prevents the graphics and user-defined plug-in code
 * from depending directly on either dlfcn(3) or the Win32 loader.
 */

#ifdef _WIN32
#include <windows.h>
typedef HMODULE ga_dlhandle;
#else
#include <dlfcn.h>
typedef void *ga_dlhandle;
#endif

#define GA_RTLD_LAZY   0x01
#define GA_RTLD_GLOBAL 0x02

ga_dlhandle ga_dlopen(const char *filename, int flags);
void *ga_dlsym(ga_dlhandle handle, const char *symbol);
const char *ga_dlerror(void);
int ga_dlclose(ga_dlhandle handle);

#endif
