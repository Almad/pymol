/* 
A* -------------------------------------------------------------------
B* This file contains source code for the PyMOL computer program
C* copyright 1998-2000 by Warren Lyford Delano of DeLano Scientific. 
D* -------------------------------------------------------------------
E* It is unlawful to modify or remove this copyright notice.
F* -------------------------------------------------------------------
G* Please see the accompanying LICENSE file for further information. 
H* -------------------------------------------------------------------
I* Additional authors of this source file include:
-* 
-* 
-*
Z* -------------------------------------------------------------------
*/
#ifndef _H_MyPNG
#define _H_MyPNG

int MyPNGWrite(char *file_name,unsigned char *p,unsigned int width,unsigned int height);
int MyPNGRead(char *file_name,unsigned char **p_ptr,unsigned int *width_ptr,unsigned int *height_ptr);

#endif
