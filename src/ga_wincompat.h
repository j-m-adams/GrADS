#ifndef GA_WINCOMPAT_H
#define GA_WINCOMPAT_H

#ifdef _WIN32
#include <io.h>
#include <stdio.h>
#include <string.h>

#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#define popen _popen
#define pclose _pclose
#define fseeko _fseeki64
#define ftello _ftelli64

#endif

#endif
