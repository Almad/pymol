#ifdef _PYMOL_NUMPY

#define NUMERIC
#include "openglutil.c"

#else
#ifdef WIN32
#include<windows.h>
#endif
#include<Python.h>
DL_EXPORT(void)
initopenglutil_num(void) {};
#endif
