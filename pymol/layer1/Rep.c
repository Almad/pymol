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

#include"os_std.h"
#include"os_gl.h"

#include"main.h"
#include"Rep.h"
#include"MemoryDebug.h"
#include"CoordSet.h"
/*========================================================================*/

void RepRenderBox(struct Rep *this,CRay *ray,Pickable **pick);
void RepInvalidate(struct Rep *I,struct CoordSet *cs,int level);
struct Rep *RepUpdate(struct Rep *I,struct CoordSet *cs,int rep);
struct Rep *RepRebuild(struct Rep *I,struct CoordSet *cs,int rep);

/*========================================================================*/
struct Rep *RepRebuild(struct Rep *I,struct CoordSet *cs,int rep)
{
  Rep *tmp = NULL;
  if(I->fNew) {
    tmp = I->fNew(cs);
    if(tmp) {
      tmp->fNew = I->fNew;
      I->fFree(I);
    }
    else {
      cs->Active[rep] = false; /* keep the old object around, but inactive */
      tmp=I;
    }
  } else 
    I->fFree(I); 
  return(tmp);
}
/*========================================================================*/
struct Rep *RepUpdate(struct Rep *I,struct CoordSet *cs,int rep)
{
  if(I->MaxInvalid) {
    if(I->MaxInvalid<=cRepInvColor) {
      if(I->fRecolor) {
        I->fRecolor(I,cs);
      } else if(I->fSameVis) {
        if(!I->fSameVis(I,cs)) {
          I=I->fRebuild(I,cs,rep);
        }
      } else 
        I=I->fRebuild(I,cs,rep);
    } else if(I->MaxInvalid<=cRepInvVisib) {
      if(I->fSameVis) {
        if(!I->fSameVis(I,cs))
          I=I->fRebuild(I,cs,rep);
      } else 
        I=I->fRebuild(I,cs,rep);
    } else 
      I=I->fRebuild(I,cs,rep);    
    if(I)
      I->MaxInvalid=0;
  }
  return(I);
}
/*========================================================================*/
void RepInvalidate(struct Rep *I,struct CoordSet *cs,int level)
{
  if(level>I->MaxInvalid) I->MaxInvalid=level;
}
/*========================================================================*/
void RepInit(Rep *I)
{
  I->fInvalidate = RepInvalidate;
  I->fUpdate = RepUpdate;
  I->fRender = RepRenderBox;
  I->fRebuild = RepRebuild;
  I->fRecolor = NULL;
  I->fSameVis = NULL;
  I->fNew = NULL;
  I->P=NULL;
  I->MaxInvalid = 0;
}
/*========================================================================*/
void RepFree(Rep *I)
{
  FreeP(I->P);
}
/*========================================================================*/
void RepRenderBox(struct Rep *this,CRay *ray,Pickable **pick)
{
  if(PMGUI) {
    glBegin(GL_LINE_LOOP);
    glVertex3i(-0.5,-0.5,-0.5);
    glVertex3i(-0.5,-0.5, 0.5);
    glVertex3i(-0.5, 0.5, 0.5);
    glVertex3i(-0.5, 0.5,-0.5);
    
    glVertex3i( 0.5, 0.5,-0.5);
    glVertex3i( 0.5, 0.5, 0.5);
    glVertex3i( 0.5,-0.5, 0.5);
    glVertex3i( 0.5,-0.5,-0.5);
    glEnd();
    
    glBegin(GL_LINES);
    glVertex3i(0,0,0);
    glVertex3i(1,0,0);
    
    glVertex3i(0,0,0);
    glVertex3i(0,2,0);
    
    glVertex3i(0,0,0);
    glVertex3i(0,0,3);
    
    glEnd();
  }

}





