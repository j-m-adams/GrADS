/* Copyright (C) 1988-2022 by George Mason University. See file COPYRIGHT. */

#include "ga_dynload.h"

#ifdef _WIN32

#include <stdio.h>

static char ga_dlerror_buffer[512];

static void ga_set_dlerror(const char *operation) {
  DWORD error = GetLastError();
  char message[384] = "unknown error";
  DWORD length;

  length = FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM |
                          FORMAT_MESSAGE_IGNORE_INSERTS,
                          NULL, error, 0, message, (DWORD)sizeof(message), NULL);
  if (length == 0) snprintf(message, sizeof(message), "Win32 error %lu", (unsigned long)error);
  snprintf(ga_dlerror_buffer, sizeof(ga_dlerror_buffer), "%s: %s", operation, message);
}

ga_dlhandle ga_dlopen(const char *filename, int flags) {
  ga_dlhandle handle;
  (void)flags;
  ga_dlerror_buffer[0] = '\0';
  handle = LoadLibraryA(filename);
  if (handle == NULL) ga_set_dlerror("LoadLibrary");
  return handle;
}

void *ga_dlsym(ga_dlhandle handle, const char *symbol) {
  FARPROC address;
  ga_dlerror_buffer[0] = '\0';
  address = GetProcAddress(handle, symbol);
  if (address == NULL) ga_set_dlerror("GetProcAddress");
  return (void *)address;
}

const char *ga_dlerror(void) {
  return ga_dlerror_buffer[0] == '\0' ? NULL : ga_dlerror_buffer;
}

int ga_dlclose(ga_dlhandle handle) {
  ga_dlerror_buffer[0] = '\0';
  if (FreeLibrary(handle)) return 0;
  ga_set_dlerror("FreeLibrary");
  return -1;
}

#else

ga_dlhandle ga_dlopen(const char *filename, int flags) {
  int native_flags = 0;
  if (flags & GA_RTLD_LAZY) native_flags |= RTLD_LAZY;
  if (flags & GA_RTLD_GLOBAL) native_flags |= RTLD_GLOBAL;
  return dlopen(filename, native_flags);
}

void *ga_dlsym(ga_dlhandle handle, const char *symbol) {
  return dlsym(handle, symbol);
}

const char *ga_dlerror(void) {
  return dlerror();
}

int ga_dlclose(ga_dlhandle handle) {
  return dlclose(handle);
}

#endif
