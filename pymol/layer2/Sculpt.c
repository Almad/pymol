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

#include"os_predef.h"
#include"os_std.h"
#include"os_gl.h"
#include"OOMac.h"
#include"Feedback.h"
#include"Util.h"
#include"Sculpt.h"
#include"SculptCache.h"
#include"Scene.h"
#include"Vector.h"
#include"Word.h"

#include"CGO.h"

#ifndef R_SMALL8
#define R_SMALL8 0.00000001
#endif

#define NB_HASH_SIZE 262144
#define EX_HASH_SIZE 65536

#define nb_hash(v) \
(((((int)*(v  ))>> 2)&0x0003F)|\
 ((((int)*(v+1))<< 4)&0x00FC0)|\
 ((((int)*(v+2))<<10)&0x3F000))

#define nb_hash_off_i0(v0i,d) \
  ((((d)+v0i)>> 2)&0x0003F)

#define nb_hash_off_i1(v1i,e) \
 ((((e)+v1i)<< 4)&0x00FC0)

#define nb_hash_off_i2(v2i,f) \
 ((((f)+v2i)<<10)&0x3F000)

#define nb_hash_off(v,d,e,f) \
(((((d)+(int)*(v  ))>> 2)&0x0003F)|\
 ((((e)+(int)*(v+1))<< 4)&0x00FC0)|\
 ((((f)+(int)*(v+2))<<10)&0x3F000))

/* below are empirically optimized */

#define ex_hash_i0(a) \
 (((a)^((a)>>5))&0x00FF)

#define ex_hash_i1(b) \
 ((  ((b)<<5))&0xFF00)

#define ex_hash(a,b) \
(((((a)^((a)>>5)))&0x00FF)|\
 (((    ((b)<<5)))&0xFF00))

#ifdef _PYMOL_INLINE
__inline__
#endif
static float ShakerDoDist(float target,float *v0,float *v1,float *d0to1,float *d1to0,float wt)
{
  float d[3],push[3];
  register float len,dev,dev_2,sc,result;

  subtract3f(v0,v1,d);
  len = (float)length3f(d);
  dev = target-len;
  if((result=(float)fabs(dev))>R_SMALL8) {
    dev_2 = wt*dev/2.0F;
    if(len>R_SMALL8) { /* nonoverlapping */
      sc = dev_2/len;
      scale3f(d,sc,push);
      add3f(push,d0to1,d0to1);
      subtract3f(d1to0,push,d1to0);
    } else { /* overlapping, so just push along X */
      float rd[3];
      get_random3f(rd);
      d0to1[0]-=rd[0]*dev_2;
      d1to0[0]+=rd[0]*dev_2;
      d0to1[1]-=rd[1]*dev_2;
      d1to0[1]+=rd[1]*dev_2;
      d0to1[2]-=rd[2]*dev_2;
      d1to0[2]+=rd[2]*dev_2;
    }
  } else
    result = 0.0;
  return result;
}

#ifdef _PYMOL_INLINE
__inline__
#endif
static float ShakerDoTors(int type,float *v0,float *v1,float *v2,float *v3,
                   float *p0,float *p1,float *p2,float *p3,float tole,float wt)
{
  
  float push0[3],push3[3];
  float axis[3],seg0[3],seg1[3],perp0[3],perp1[3];
  float dir[3];
  float sc;
  float sign,dp;
  float result = 0.0F;
  
  /* v0       v3
     \      /
     v1__v2 */
  
  subtract3f(v2,v1,axis);    
  subtract3f(v0,v1,seg0);
  subtract3f(v3,v2,seg1);
  cross_product3f(seg0,axis,perp0);
  cross_product3f(axis,seg1,perp1);
  
  normalize3f(perp0);
  normalize3f(perp1);

#if 0
  {
    float vert[3];
    /* debuggin */
    
    CGOColor(DebugCGO,1.0,0.0,0.0);
    CGOBegin(DebugCGO,GL_LINES);
    CGOVertexv(DebugCGO,v1);
    add3f(perp0,v1,vert);
    CGOVertexv(DebugCGO,vert);
    
    CGOColor(DebugCGO,0.0,1.0,1.0);
    CGOVertexv(DebugCGO,v2);
    add3f(perp1,v2,vert);
    CGOVertexv(DebugCGO,vert);
    CGOEnd(DebugCGO);
  }
#endif

  dp = dot_product3f(perp0,perp1);

  switch(type) {
  case cShakerTorsSP3SP3:
    if(dp<-0.5F) {
      result = ((float)fabs(dp))-0.5F;
      if(result<tole) /* discontinuous low bottom well */
        result = result/5.0F;       
    } else if(dp<0.5) {
      result = -0.5F-dp;
      if(result<tole) /* discontinuous low bottom well */
        result = result;       
    } else {
      result = 1.0F-dp;
    }
    break;
  case cShakerTorsAmide: 
    if(dp>-0.7F) {
      result = 1.0F-dp;
    } else {
      result = -1.0F-dp;
    }
    result *= 50.0F; /* emphasize */
    break;
  case cShakerTorsDisulfide:
    if(fabs(dp)<tole)
      return 0.0F;
    result = -dp;
    if(result<tole)
      result = result/25.F;             
    break;
  }


  cross_product3f(perp0,perp1,dir);
  sign = dot_product3f(axis,dir);
  
  if(sign<0.0F)
    sc=wt*result;
  else
    sc=-wt*result;
  
  scale3f(perp0,sc,push0);
  scale3f(perp1,sc,push3);
  
  add3f(p0,push0,p0);
  add3f(p3,push3,p3);
  subtract3f(p1,push0,p1);
  subtract3f(p2,push3,p2);

#if 0
    {
      float vert[3];
      /* debuggin */
      
      CGOColor(DebugCGO,1.0,0.5,0.0);
      CGOBegin(DebugCGO,GL_LINES);
      CGOLinewidth(DebugCGO,3.0);
      
      /* draw from */
      
      CGOVertexv(DebugCGO,v0);
      add3f(v0,push0,vert);
      CGOVertexv(DebugCGO,vert);
      
      /* draw to */
      
      CGOVertexv(DebugCGO,v3);
      add3f(v3,push3,vert);
      CGOVertexv(DebugCGO,vert);
      CGOEnd(DebugCGO);              
    }    
#endif

  return result;
  
}

#ifdef _PYMOL_INLINE
__inline__
#endif
static float ShakerDoDistLimit(float target,float *v0,float *v1,float *d0to1,float *d1to0,float wt)
{
  float d[3],push[3];
  register float len,dev,dev_2,sc;

  subtract3f(v0,v1,d);
  len = (float)length3f(d);
  dev = len-target;
  if(dev>0.0F) { /* assuming len is non-zero since it is above target */
    dev_2 = wt*dev*(-0.5F);
    sc = dev_2/len;
    scale3f(d,sc,push);
    add3f(push,d0to1,d0to1);
    subtract3f(d1to0,push,d1to0);
    return dev;
  } else {
    return 0.0F;
  }
}

#ifdef _PYMOL_INLINE
__inline__
#endif
static float ShakerDoDistMinim(float target,float *v0,float *v1,float *d0to1,float *d1to0,float wt)
{
  float d[3],push[3];
  register float len,dev,dev_2,sc;

  subtract3f(v0,v1,d);
  len = (float)length3f(d);
  dev = len-target;

  if(dev<0.0F) { /* assuming len is non-zero since it is above target */
    dev_2 = -wt*dev*0.5F;
    sc = dev_2/len;
    scale3f(d,sc,push);
    add3f(push,d0to1,d0to1);
    subtract3f(d1to0,push,d1to0);
    return -dev;
  } else {
    return 0.0F;
  }
}


CSculpt *SculptNew(PyMOLGlobals *G)
{
  OOAlloc(G,CSculpt);
  I->G=G;
  I->Shaker = ShakerNew(G);
  I->NBList = VLAlloc(int,150000);
  I->NBHash = Alloc(int,NB_HASH_SIZE);
  I->EXList = VLAlloc(int,100000);
  I->EXHash = Alloc(int,EX_HASH_SIZE);
  I->Don = VLAlloc(int,1000);
  I->Acc = VLAlloc(int,1000);
  { 
    int a;
    for(a=1;a<256;a++)
      I->inverse[a] = 1.0F/a;
  }
  return(I);
}

typedef struct {
  int *neighbor;
  AtomInfoType *ai;
  int *atm2idx1,*atm2idx2;
} CountCall;

typedef struct {
  PyMOLGlobals *G;
  CShaker *Shaker;
  AtomInfoType *ai;
  int *atm2idx;
  CoordSet *cSet, **discCSet;
  float *coord;
  int *neighbor;
  int atom0;
  int min,max,mode;
} ATLCall;

static int count_branch(CountCall *CNT, int atom, int limit)
{
  AtomInfoType *ai = CNT->ai + atom;
  int count = 0;

  if(!ai->temp1) {
    count = (ai->hydrogen ? 0 : 1);
    if(count) {
      if((CNT->atm2idx1[atom]<0)||(CNT->atm2idx2[atom]<0))
        count = 0;
    }
    if(count && (limit>0)) {
      int n0 = CNT->neighbor[atom]+1;
      int b1;
      ai->temp1 = true;
      while( (b1 = CNT->neighbor[n0]) >=0 ) {
        count += count_branch(CNT, b1, limit-1);
        n0+=2;
      }
      ai->temp1 = false;
    }
  }
  return count;
}

static void add_triangle_limits(ATLCall *ATL, int prev, int cur, float dist, int count)
{
  register ATLCall *I = ATL;
  int n0;
  int n1;
  float dist_limit;
  int atom1;

  n0 = I->neighbor[cur];  
  if((count>=I->min) && (count>1)) {
    register int add_flag = false;
    switch(I->mode) {
    case 1: 
      add_flag = 1; /* all */
      break;
    case 2:
      add_flag = (count && !(count&1)); /* evens */
      break;
    case 3:
      add_flag = ( (count & (count-1))==0); /* powers of two */
      break;
    case 0:
    default:
      add_flag = (!I->ai[I->atom0].hydrogen); /* all heavies */
      break;
    }
    if(add_flag) {
      n1 = n0+1;
      
      /* first mark and register */
      while( (atom1 = I->neighbor[n1]) >=0 ) {
        if((!I->ai[atom1].temp1)&&(I->atom0<atom1)) {
          register int ref = prev;
          if(count&0x1) { /* odd */
            ref = cur;
          }
          if(((!I->discCSet)||
	      ((I->cSet==I->discCSet[ref])&&(I->cSet==I->discCSet[atom1]))) &&
	     ((I->mode!=0)||(!I->ai[atom1].hydrogen))) {
            register int ia = I->atm2idx[ref];
            register int ib = I->atm2idx[atom1];
            if((ia>=0)&&(ib>=0)) {
              register float *va = I->coord + 3*ia;
              register float *vb = I->coord + 3*ib;
              dist_limit = dist + diff3f(va,vb);
              ShakerAddDistCon(I->Shaker,I->atom0,atom1,dist_limit,cShakerDistLimit,1.0F);
            }
          }
          I->ai[atom1].temp1 = 1;
        }
        n1+=2;
      }
    }
  }

  if(count<=I->max) {
    /* then recurse */
    n1 = n0+1;
    while( (atom1 = I->neighbor[n1]) >=0 ) {
      if(I->ai[atom1].temp1<2) {
        dist_limit = dist;
        if(!(count&0x1)) { /* accumulate distances between even atoms only */
          if((!I->discCSet)||((I->cSet==I->discCSet[prev])&&(I->cSet==I->discCSet[atom1]))) {
            register int ia = I->atm2idx[prev];
            register int ib = I->atm2idx[atom1];
            if((ia>=0)&&(ib>=0)) {
              register float *va = I->coord + 3*ia;
              register float *vb = I->coord + 3*ib;
              dist_limit += diff3f(va,vb);
            }
          }
          I->ai[atom1].temp1 = 2;
        }
        I->ai[atom1].temp1=2;
        add_triangle_limits(I, cur, atom1, dist_limit, count+1);
      }
      n1+=2;
    }
  }
}

void SculptMeasureObject(CSculpt *I,ObjectMolecule *obj,int state,int match_state)
{
  PyMOLGlobals *G=I->G;
  int a,a0,a1,a2,a3,b0,b1,b2,b3;
  BondType *b;
  float *v0,*v1,*v2,*v3,d,dummy;
  CoordSet *cs;
  int n0,n1,n2;
  int *planer = NULL;
  int *linear = NULL;
  int *single = NULL;
  int nex = 1;
  int *j,*k,xhash;
  int ex_type;
  AtomInfoType *ai,*ai1,*ai2,*oai;
  int xoffset;
  int use_cache = 1;
  
  PRINTFD(G,FB_Sculpt)
    " SculptMeasureObject-Debug: entered.\n"
    ENDFD;

  if(match_state==-1)
    match_state = state;

  ShakerReset(I->Shaker);

  UtilZeroMem(I->NBHash,NB_HASH_SIZE*sizeof(int));
  UtilZeroMem(I->EXHash,EX_HASH_SIZE*sizeof(int));

  if(state<obj->NCSet)
    if(obj->CSet[state])
      {
        oai = obj->AtomInfo;

        VLACheck(I->Don,int,obj->NAtom);
        VLACheck(I->Acc,int,obj->NAtom);
        ai = obj->AtomInfo;
        for(a=0;a<obj->NAtom;a++) {
          I->Don[a]=false;
          I->Acc[a]=false;
          AtomInfoCheckUniqueID(G,ai);
          ai++;
        }
        
        ObjectMoleculeVerifyChemistry(obj);
        ObjectMoleculeUpdateNeighbors(obj);

        cs = obj->CSet[state];

        use_cache = SettingGet_i(G,cs->Setting,obj->Obj.Setting,cSetting_sculpt_memory);
        if(obj->NBond) {

          planer=Alloc(int,obj->NAtom);
          linear=Alloc(int,obj->NAtom);
          single=Alloc(int,obj->NAtom);
          ai = obj->AtomInfo;
          for(a=0;a<obj->NAtom;a++) {
            planer[a]=(ai->geom==cAtomInfoPlaner);
            linear[a]=(ai->geom==cAtomInfoLinear);
            single[a]=(ai->geom==cAtomInfoSingle);
            ai++;
          }

          /* brain-dead donor/acceptor assignment
           * REPLACE later on with pattern-based system */


          /* pass 1 */

          b=obj->Bond;
          for(a=0;a<obj->NBond;a++)
            {
              b1 = b->index[0];
              b2 = b->index[1];
              
              ai1=obj->AtomInfo+b1;
              ai2=obj->AtomInfo+b2;

              /* make blanket assumption that all nitrogens with 
                 <3 bonds are donors -- we qualify this below...*/
              
              if(ai1->protons==cAN_N) {
                n1 = obj->Neighbor[b1];
                if(obj->Neighbor[n1]<3) { /* N with L.P. */
                  I->Don[b1]=true;
                }
              }

              if(ai2->protons==cAN_N) {
                n2 = obj->Neighbor[b2];
                if(obj->Neighbor[n2]<3) { /* N with L.P. */
                  I->Don[b2]=true;
                }
              }

              /* assume O is always an acceptor...*/
              
              if(ai1->protons==cAN_O) I->Acc[b1]=true;
              if(ai2->protons==cAN_O) I->Acc[b2]=true;
              b++;
            }

          /* pass 2 */
          b=obj->Bond;             
          for(a=0;a<obj->NBond;a++)
            {
              b1 = b->index[0];
              b2 = b->index[1];

              /* nitrogens with lone pairs are acceptors 
                 (not donors as assumed above) */
              
              ai1=obj->AtomInfo+b1;
              ai2=obj->AtomInfo+b2;
              
              if(ai1->protons==cAN_N) {
                if(b->order==2) {
                  n1 = obj->Neighbor[b1];
                  if(obj->Neighbor[n1]<3) { /* N with L.P. */
                    I->Acc[b1]=true;
                    I->Don[b1]=false;
                  }
                }
              }
              if(ai2->protons==cAN_N) {
                if(b->order==2) {
                  n2 = obj->Neighbor[b2];
                  if(obj->Neighbor[n2]<3) { /* N with L.P. */
                    I->Acc[b2]=true;
                    I->Don[b2]=false;
                  }
                }
              }
              b++;
            }

          /* pass 3 */
          b=obj->Bond;
          for(a=0;a<obj->NBond;a++)
            {
              b1 = b->index[0];
              b2 = b->index[1];
              
              ai1=obj->AtomInfo+b1;
              ai2=obj->AtomInfo+b2;
                     
              /* however, every NH is a donor, 
                 even if it's SP2 */
              
              if(ai1->protons==cAN_H) {
                
                /* donors: any H attached to O, N */
                switch(ai2->protons) {
                case cAN_O: 
                  I->Don[b1]=true; 
                  I->Don[b2]=true; /* mark heavy atom too... */
                  break;
                case cAN_N: 
                  I->Don[b1]=true; 
                  I->Don[b2]=true;
                  break;
                }
              } else if(ai2->protons==cAN_H) {
                switch(ai1->protons) {
                case cAN_O: 
                  I->Don[b1]=true; 
                  I->Don[b2]=true; /* mark heavy atom too... */
                  break;
                case cAN_N: 
                  I->Don[b1]=true; 
                  I->Don[b2]=true; /* mark heavy atom too... */
                  break;
                }
              }

              b++;
            }

          /* atom pass */
          ai1 = obj->AtomInfo;
          for(a=0;a<obj->NAtom;a++) {
            /* make sure all nonbonded atoms get categorized */

            n0 = obj->Neighbor[a];
            if(obj->Neighbor[n0]==0) { /* nonbonded */
              if(ai1->protons==cAN_O) {
                I->Don[a] = true;
                I->Acc[a] = true;
              } else if(ai1->protons==cAN_N) {
                I->Don[a] = true;
              } 
            }
            /*            
            if(I->Acc[a]) {
              printf("ACC %s %s %s\n",ai1->chain,ai1->resi,ai1->name);
            }
            if(I->Don[a]) {
              printf("DON %s %s %s\n",ai1->chain,ai1->resi,ai1->name);
              }*/

            ai1++;
          }
          
          /*  exclusions */
          b=obj->Bond;          
          for(a=0;a<obj->NBond;a++)
            {
              b1 = b->index[0];
              b2 = b->index[1];
              
              ai1=obj->AtomInfo+b1;
              ai2=obj->AtomInfo+b2;
          
              xhash = ( (b2>b1) ? ex_hash(b1,b2) : ex_hash(b2,b1));
              VLACheck(I->EXList,int,nex+3);
              j = I->EXList+nex;
              *(j++)=*(I->EXHash+xhash);
              if(b2>b1) {
                *(j++)=b1;
                *(j++)=b2;
              } else {
                *(j++)=b2;
                *(j++)=b1;
              }
              *(j++)=2; /* 1-2 exclusion */
              *(I->EXHash+xhash)=nex;
              nex+=4;


              if(obj->DiscreteFlag) {
                if((cs==obj->DiscreteCSet[b1])&&(cs==obj->DiscreteCSet[b2])) {
                  a1=obj->DiscreteAtmToIdx[b1];
                  a2=obj->DiscreteAtmToIdx[b2];
                } else {
                  a1=-1;
                  a2=-1;
                }
              } else {
                a1=cs->AtmToIdx[b1];
                a2=cs->AtmToIdx[b2];
              }
              if((a1>=0)&&(a2>=0))
                {
                  v1 = cs->Coord+3*a1;
                  v2 = cs->Coord+3*a2;
                  d = (float)diff3f(v1,v2);
                  if(use_cache) {
                    if(!SculptCacheQuery(G,cSculptBond,
                                         oai[b1].unique_id,
                                         oai[b2].unique_id,0,0,&d))
                      SculptCacheStore(G,cSculptBond,
                                       oai[b1].unique_id,
                                       oai[b2].unique_id,0,0,d);
                  }
                  ShakerAddDistCon(I->Shaker,b1,b2,d,cShakerDistBond,1.0F); 
                  /* NOTE: storing atom indices, not coord. ind.! */
                }
              b++;
            }

          /* triangle relationships */
          {
            ATLCall atl;
            ai1 = obj->AtomInfo;

            atl.G = I->G;
            atl.Shaker = I->Shaker;
            atl.ai = obj->AtomInfo;
            atl.cSet = cs;
            
            if(obj->DiscreteFlag) {
              atl.atm2idx = obj->DiscreteAtmToIdx;
              atl.discCSet = obj->DiscreteCSet;
            } else {
              atl.atm2idx = cs->AtmToIdx;
              atl.discCSet = NULL;
            }
            atl.coord = cs->Coord;
            atl.neighbor = obj->Neighbor;
            atl.min =  SettingGet_i(G,cs->Setting,obj->Obj.Setting,cSetting_sculpt_tri_min);
            atl.max = SettingGet_i(G,cs->Setting,obj->Obj.Setting,cSetting_sculpt_tri_max);
            atl.mode = SettingGet_i(G,cs->Setting,obj->Obj.Setting,cSetting_sculpt_tri_mode); 

            for(a=0;a<obj->NAtom;a++) {
              
              atl.atom0 = a;

              /* clear the flag -- TODO replace with array */
              {
                int aa;
                ai = obj->AtomInfo;
                for(aa=0;aa<obj->NAtom;aa++) {
                  ai->temp1 = false;
                  ai++;
                }
              }
              
              ai1->temp1 = true;
              add_triangle_limits(&atl, a, a, 0.0F, 1);
              ai1++;
            }
          }

          /* if we have a match state, establish minimum distances */
          if((match_state>=0)&&(match_state<obj->NCSet)&&(!obj->DiscreteFlag)) {
            CoordSet *cs2 = obj->CSet[match_state];
            int n_site = 0;
            if(cs2) {
              float minim_min =  SettingGet_f(G,cs->Setting,obj->Obj.Setting,cSetting_sculpt_min_min);
              float minim_max = SettingGet_f(G,cs->Setting,obj->Obj.Setting,cSetting_sculpt_min_max);
              float maxim_min =  SettingGet_f(G,cs->Setting,obj->Obj.Setting,cSetting_sculpt_max_min);
              float maxim_max = SettingGet_f(G,cs->Setting,obj->Obj.Setting,cSetting_sculpt_max_max);

              int *site = Calloc(int,obj->NAtom);            
              float *weight = Calloc(float,obj->NAtom);
              /* first, find candidate atoms with sufficient connectivity */
              CountCall cnt;

              cnt.ai = obj->AtomInfo;
              cnt.neighbor = obj->Neighbor;
              cnt.atm2idx1 = cs->AtmToIdx;
              cnt.atm2idx2 = cs2->AtmToIdx;

              {
                int aa;
                ai = obj->AtomInfo;
                for(aa=0;aa<obj->NAtom;aa++) {
                  ai->temp1 = false;
                  ai++;
                }
              }
              
              ai1 = obj->AtomInfo;
              for(b0=0;b0<obj->NAtom;b0++) {
                int n_qual_branch = 0,cb;
                int adj_site = false;
                ai1->temp1 = true;
                n0 = obj->Neighbor[b0]+1;
                while( (b1 = obj->Neighbor[n0]) >=0) {
                  if(site[b1]) {
                    adj_site = true;
                    break;
                  }
                  cb = count_branch(&cnt, b1, 3);
                  if(cb>3) {
                    n_qual_branch++;
                  }
                  n0+=2;
                }
                ai1->temp1 = false;
                if((n_qual_branch>2)&&(!adj_site)) {
                  site[b0] = 10;
                } else if(!adj_site) {
                  switch(ai1->name[0]) {
                  case 'O':
                    if(!ai1->name[1])
                      if(AtomInfoKnownPolymerResName(G,ai1->resn)) site[b0] = 40;  /* main-chain carbonyl */
                    break;
                  case 'C':
                    switch(ai1->name[1]) {
                    case 'Z':
                      switch(ai1->name[2]) {
                      case 0:
                        if(strcmp(ai1->resn,"ARG")==0) site[b0] = 20;  /* ARG/CZ */
                        else if(strcmp(ai1->resn,"TYR")==0) site[b0] = 20; /* TYR/CZ */
                        else if(strcmp(ai1->resn,"PHE")==0) site[b0] = 20; /* PHE/CZ */
                        break;
                      }
                      break;
                    case 'E':
                      switch(ai1->name[2]) {
                      case 0:
                        if(strcmp(ai1->resn,"LYS")==0) site[b0] = 20;  /* LYS/CE */
                        break;
                      }
                      break;
                    case 'D':
                      switch(ai1->name[2]) {
                      case 0:
                        if(strcmp(ai1->resn,"GLU")==0) site[b0] = 20;  /* GLU/CD */
                        else if(strcmp(ai1->resn,"GLN")==0) site[b0] = 20;  /* GLN/CD */
                        break;
                      }
                      break;
                    case 'G':
                      switch(ai1->name[2]) {
                      case 0:
                        if(strcmp(ai1->resn,"LEU")==0) site[b0] = 20;  /* LEU/CG */
                        else if(strcmp(ai1->resn,"ASP")==0) site[b0] = 20;  /* ASP/CG */
                        else if(strcmp(ai1->resn,"ASN")==0) site[b0] = 20;  /* ASN/CG */
                        break;
                      }
                      break;
                    }
                    break;
                  case 'S':
                    switch(ai1->name[1]) {
                    case 'D':
                      switch(ai1->name[2]) {
                      case 0:
                        if(strcmp(ai1->resn,"MET")==0) site[b0] = 20;  /* MET/SD */
                        break;
                      }
                      break;
                    }
                    break;
                  }
                }
                ai1++;
              }
              
              for(b0=0;b0<obj->NAtom;b0++) {
                if(site[b0]) {
                  weight[n_site]=10.0F/site[b0];
                  site[n_site] = b0;
                  n_site++;
                }
              }

              {
                
                for(a0=0;a0<n_site;a0++) {
                  for(a1=a0+1;a1<n_site;a1++) {
                    float wt = weight[a0]*weight[a1];
                    b0 = site[a0];
                    b1 = site[a1];
                    
                    {
                      int i0a = cs->AtmToIdx[b0];
                      int i1a = cs->AtmToIdx[b1];
                      int i0b = cs2->AtmToIdx[b0];
                      int i1b = cs2->AtmToIdx[b1];
                      
                      if((i0a>=0)&&(i1a>=0)&&(i0b>=0)&&(i1b>=0)) {
                        float *v0a = cs->Coord + 3*i0a;
                        float *v1a = cs->Coord + 3*i1a;
                        float *v0b = cs2->Coord + 3*i0b;
                        float *v1b = cs2->Coord + 3*i1b;
                        float dist0,dist1,min_dist,max_dist;
                        dist0 = diff3f(v0a,v1a);
                        dist1 = diff3f(v0b,v1b);
                        min_dist = (dist0<dist1) ? dist0 : dist1;
                        if((min_dist>=minim_min)&&(min_dist<=minim_max)) {
                          ShakerAddDistCon(I->Shaker,b0,b1,min_dist,cShakerDistMinim,wt);
                        }
                        max_dist = (dist0>dist1) ? dist0 : dist1;
                        if((max_dist>=maxim_min)&&(max_dist<=maxim_max)) {
                          ShakerAddDistCon(I->Shaker,b0,b1,max_dist,cShakerDistMaxim,wt);
                        }
                      }
                    }
                  }
                }
              }
                FreeP(weight);
              FreeP(site);
            }
          }
          /* now pick up those 1-3 interations */
          
          /* b1-b0-b2 */

          for(b0=0;b0<obj->NAtom;b0++) {
            n0 = obj->Neighbor[b0]+1;
            while(obj->Neighbor[n0]>=0) {
              b1 = obj->Neighbor[n0];
              n1 = n0+2;
              while(obj->Neighbor[n1]>=0) {
                b2 = obj->Neighbor[n1];


                xhash = ( (b2>b1) ? ex_hash(b1,b2) : ex_hash(b2,b1));
                VLACheck(I->EXList,int,nex+3);
                j = I->EXList+nex;
                *(j++)=*(I->EXHash+xhash);
                if(b2>b1) {
                  *(j++)=b1;
                  *(j++)=b2;
                } else {
                  *(j++)=b2;
                  *(j++)=b1;
                }
                *(j++)=3; /* 1-3 exclusion */
                *(I->EXHash+xhash)=nex;
                nex+=4;

                if(obj->DiscreteFlag) {
                  if((cs==obj->DiscreteCSet[b0])&&(cs==obj->DiscreteCSet[b1])&&(cs==obj->DiscreteCSet[b2])) {
                    a0=obj->DiscreteAtmToIdx[b0];
                    a1=obj->DiscreteAtmToIdx[b1];
                    a2=obj->DiscreteAtmToIdx[b2];
                  } else {
                    a0=-1;
                    a1=-1;
                    a2=-1;
                  }
                } else {
                  a0=cs->AtmToIdx[b0];
                  a1=cs->AtmToIdx[b1];
                  a2=cs->AtmToIdx[b2];
                }

                if((a0>=0)&&(a1>=0)&&(a2>=0)) {
                  v1 = cs->Coord+3*a1;
                  v2 = cs->Coord+3*a2;
                  d = (float)diff3f(v1,v2);
                  if(use_cache) {
                    if(!SculptCacheQuery(G,cSculptAngl,
                                         oai[b0].unique_id,
                                         oai[b1].unique_id,
                                         oai[b2].unique_id,0,&d))
                      SculptCacheStore(G,cSculptAngl,
                                       oai[b0].unique_id,
                                       oai[b1].unique_id,
                                       oai[b2].unique_id,0,d);
                  }
                  ShakerAddDistCon(I->Shaker,b1,b2,d,cShakerDistAngle,1.0F); 


                  if(linear[b0]&&(linear[b1]||linear[b2])) {
                    
                    if(use_cache) {
                      if(!SculptCacheQuery(G,cSculptLine,
                                           oai[b1].unique_id,
                                           oai[b0].unique_id,
                                           oai[b2].unique_id,0,&dummy))
                        SculptCacheStore(G,cSculptLine,
                                         oai[b1].unique_id,
                                         oai[b0].unique_id,
                                         oai[b2].unique_id,0,0.0);
                    }
                    ShakerAddLineCon(I->Shaker,b1,b0,b2); 
                  }
                }
                n1+=2;
              }
              n0+=2;
            }
          }

          /* and record the pyramidal and planer geometries */
          
          for(b0=0;b0<obj->NAtom;b0++) {
            n0 = obj->Neighbor[b0]+1;
            while(obj->Neighbor[n0]>=0) {
              b1 = obj->Neighbor[n0];
              n1 = n0+2;
              while(obj->Neighbor[n1]>=0) {
                b2 = obj->Neighbor[n1];
                n2 = n1+2;
                while(obj->Neighbor[n2]>=0) {
                  b3 = obj->Neighbor[n2];
                  
                                
                  if(obj->DiscreteFlag) {
                    if((cs==obj->DiscreteCSet[b0])&&
                       (cs==obj->DiscreteCSet[b1])&&
                       (cs==obj->DiscreteCSet[b2])) {
                      a0=obj->DiscreteAtmToIdx[b0];
                      a1=obj->DiscreteAtmToIdx[b1];
                      a2=obj->DiscreteAtmToIdx[b2];
                      a3=obj->DiscreteAtmToIdx[b3];
                    } else {
                      a0=-1;
                      a1=-1;
                      a2=-1;
                      a3=-1;
                    }
                  } else {
                    a0=cs->AtmToIdx[b0];
                    a1=cs->AtmToIdx[b1];
                    a2=cs->AtmToIdx[b2];
                    a3=cs->AtmToIdx[b3];
                  }
                  if((a0>=0)&&(a1>=0)&&(a2>=0)&&(a3>=0)) {
                    v0 = cs->Coord+3*a0;
                    v1 = cs->Coord+3*a1;
                    v2 = cs->Coord+3*a2;
                    v3 = cs->Coord+3*a3;
                    d = ShakerGetPyra(v0,v1,v2,v3);
                    if(fabs(d)<0.05) {
                      planer[b0]=true;
                    }
                    if(planer[b0])
                      d=0.0;
                    if(use_cache) {
                      if(!SculptCacheQuery(G,cSculptPyra,
                                           oai[b1].unique_id,
                                           oai[b0].unique_id,
                                           oai[b2].unique_id,
                                           oai[b3].unique_id,
                                           &d))
                        SculptCacheStore(G,cSculptPyra,
                                         oai[b1].unique_id,
                                         oai[b0].unique_id,
                                         oai[b2].unique_id,
                                         oai[b3].unique_id,
                                         d);
                    }
                    ShakerAddPyraCon(I->Shaker,b0,b1,b2,b3,d); 
                  }                
                  n2+=2;
                }
                n1+=2;
              }
              n0+=2;
            }
          }

          /* b1\b0_b2/b3 */
        
          for(b0=0;b0<obj->NAtom;b0++) {
            n0 = obj->Neighbor[b0]+1;
            while((b1 = obj->Neighbor[n0])>=0) {
              n1 = obj->Neighbor[b0]+1;
              while((b2 = obj->Neighbor[n1])>=0) {
                if(b1!=b2) {
                  n2 =  obj->Neighbor[b2]+1;
                  while((b3 = obj->Neighbor[n2])>=0) {
                    if((b3!=b0)&&(b3>b1)) {
                      if(!(planer[b0]||planer[b2]||linear[b0]||linear[b2])) {
                        int type;
                        if((oai[b0].protons == cAN_S)&&
                           (oai[b2].protons == cAN_S))
                          type = cShakerTorsDisulfide;
                        else 
                          type = cShakerTorsSP3SP3;
                        ShakerAddTorsCon(I->Shaker,b1,b0,b2,b3,type);
                      } 
                      if(planer[b0]&&planer[b2]) {
                        if(((oai[b1].protons == cAN_H)&&single[b1]&&
                            (oai[b0].protons == cAN_N)&&
                            (oai[b2].protons == cAN_C)&&
                            (oai[b3].protons == cAN_O)&&planer[b3]&&
                            (!WordMatchExact(G,oai[b0].resn,"PRO",true)))||
                           ((oai[b1].protons == cAN_O)&&planer[b1]&&
                            (oai[b0].protons == cAN_C)&&
                            (oai[b2].protons == cAN_N)&&
                            (oai[b3].protons == cAN_H)&&single[b3]&&
                           (!WordMatchExact(G,oai[b2].resn,"PRO",true)))) {
                          ShakerAddTorsCon(I->Shaker,b1,b0,b2,b3,cShakerTorsAmide);
                        }
                      }
                      /* check 1-4 exclusion */
                      xhash = ex_hash(b1,b3);
                      
                      ex_type = 4; 
                      
                      xoffset = *(I->EXHash+xhash);
                      while(xoffset) {
                        k = I->EXList + xoffset;
                        if((abs(*(k+3))==4)&&
                           (*(k+1)==b1)&&
                           (*(k+2)==b3)) {
                          if((b0!=*(k+4))&&
                             (b2!=*(k+5))) {
                            if(planer[b0]&&planer[b2]&&
                               planer[*(k+4)]&&planer[*(k+5)]) {
                              /* aromatic */ 
                              *(k+3)=-4;
                            }
                          }
                          ex_type = 0; /* duplicate, skip */
                          break;
                        }
                        xoffset = *k;
                      }
                      if(ex_type) {
                        VLACheck(I->EXList,int,nex+5);
                        j = I->EXList+nex;
                        *(j++)=*(I->EXHash+xhash);
                        *(j++)=b1;
                        *(j++)=b3;
                        if(planer[b0]&&planer[b2])
                          *(j++)=-4;
                        else
                          *(j++)=ex_type;
                        *(j++)=b0;
                        *(j++)=b2;
                        *(I->EXHash+xhash)=nex;

                        nex+=6;
                      }
                      
                      /* planarity */
                      
                      if(obj->DiscreteFlag) {
                        if((cs==obj->DiscreteCSet[b0])&&
                           (cs==obj->DiscreteCSet[b1])&&
                           (cs==obj->DiscreteCSet[b2])&&
                           (cs==obj->DiscreteCSet[b3])) {
                          a0=obj->DiscreteAtmToIdx[b0];
                          a1=obj->DiscreteAtmToIdx[b1];
                          a2=obj->DiscreteAtmToIdx[b2];
                          a3=obj->DiscreteAtmToIdx[b3];
                        } else {
                          a0=-1;
                          a1=-1;
                          a2=-1;
                          a3=-1;
                        }
                      } else {
                        a0=cs->AtmToIdx[b0];
                        a1=cs->AtmToIdx[b1];
                        a2=cs->AtmToIdx[b2];
                        a3=cs->AtmToIdx[b3];
                      }
                      if((a0>=0)&&(a1>=0)&&(a2>=0)&&(a3>=0)) {
                        v0 = cs->Coord+3*a0;
                        v1 = cs->Coord+3*a1;
                        v2 = cs->Coord+3*a2;
                        v3 = cs->Coord+3*a3;
                        
                        d = 0.0;
                        if(planer[b0]&&planer[b2]) {
                          float deg = get_dihedral3f(v1,v0,v2,v3);
                          if(fabs(deg)<deg_to_rad(10.0))
                            d = 1.0; 
                          else if(fabs(deg)>deg_to_rad(170))
                            d = -1.0;
                          
                          /*
                            if(!(((oai[b1].protons == cAN_H)&&single[b1]&&
                            (oai[b0].protons == cAN_N)&&planer[b0]&&
                            (oai[b2].protons == cAN_C)&&planer[b2]&&
                            (oai[b3].protons == cAN_O)&&planer[b3])||
                            ((oai[b1].protons == cAN_O)&&planer[b1]&&
                            (oai[b0].protons == cAN_C)&&planer[b0]&&
                            (oai[b2].protons == cAN_N)&&planer[b2]&&
                            (oai[b3].protons == cAN_H)&&single[b3]))) 
                          */
                          {
                            int fixed = false;
#if 0
                            /* look for 4, 5, 6, 7, or 8 cycle that
                               connects back to b1 if found, then this
                               planer system is fixed (either at zero
                               or 180 -- it can't flip it over) */
                            {
                              int b4,b5,b6,b7,b8,b9,b10;
                              int n3,n4,n5,n6,n7,n8,n9;
                              n3 = obj->Neighbor[b2]+1;
                              while((!fixed)&&(b4 = obj->Neighbor[n3])>=0) {
                                if(b4!=b0) {
                                  n4 = obj->Neighbor[b4]+1;
                                  while((!fixed)&&(b5 = obj->Neighbor[n4])>=0) {
                                    if(b5!=b2) {
                                      n5 = obj->Neighbor[b5]+1;
                                      while((!fixed)&&(b6 = obj->Neighbor[n5])>=0) {
                                        if(b6==b0) { /* 4-cycle */
                                          fixed=true;
                                        } else if(b6!=b4) {
                                          n6 = obj->Neighbor[b6]+1;
                                          while((!fixed)&&(b7 = obj->Neighbor[n6])>=0) {
                                            if(b7==b0) {  /* 5-cycle */
                                              fixed=true;
                                            } else if(b7!=b5) {
                                              n7 = obj->Neighbor[b7]+1;
                                              while((!fixed)&&(b8 = obj->Neighbor[n7])>=0) {
                                                if(b8==b0) {  /* 6-cycle */
                                                  fixed=true;
                                                } else if(b8!=b6) {
                                                  n8 = obj->Neighbor[b8]+1;
                                                  while((!fixed)&&(b9 = obj->Neighbor[n8])>=0) {
                                                    if(b9==b0) {  /* 7-cycle */
                                                      fixed=true;
                                                    } else if(b9!=b7) {
                                                      n9 = obj->Neighbor[b9]+1;
                                                      while((!fixed)&&(b10 = obj->Neighbor[n9])>=0) {
                                                        if(b10==b0) {  /* 8-cycle */
                                                          fixed=true;
                                                        } 
                                                        n9+=2;
                                                      }
                                                    }
                                                    n8+=2;
                                                  }
                                                }
                                                n7+=2;
                                              }
                                            }
                                            n6+=2;
                                          }
                                        }
                                        n5+=2;
                                      }
                                    }
                                    n4+=2;
                                  }
                                }
                                n3+=2;
                              }
                            }
#else
                            {
                              /* b1\b0_b2/b3-b4-b5-b6-b7... */
                              
                              int b4,b5,b6,b7,b8,b9,b10;
                              int n3,n4,n5,n6,n7,n8,n9;
                              n3 = obj->Neighbor[b2]+1;
                              while((!fixed)&&(b4 = obj->Neighbor[n3])>=0) {
                                if(b4!=b0) {
                                  n4 = obj->Neighbor[b4]+1;
                                  while((!fixed)&&(b5 = obj->Neighbor[n4])>=0) {
                                    if(b5!=b2) {
                                      n5 = obj->Neighbor[b5]+1;
                                      while((!fixed)&&(b6 = obj->Neighbor[n5])>=0) {
                                        if(b6==b0) { /* 4-cycle */
                                          fixed=true;
                                        } else if((b6!=b4)&&(b6!=b2)) {
                                          n6 = obj->Neighbor[b6]+1;
                                          while((!fixed)&&(b7 = obj->Neighbor[n6])>=0) {
                                            if(b7==b0) {  /* 5-cycle */
                                              fixed=true;
                                            } else if((b7!=b5)&&(b7!=b2)) {
                                              n7 = obj->Neighbor[b7]+1;
                                              while((!fixed)&&(b8 = obj->Neighbor[n7])>=0) {
                                                if(b8==b0) {  /* 6-cycle */
                                                  fixed=true;
                                                } else if((b8!=b6)&&(b8!=b2)) {
                                                  n8 = obj->Neighbor[b8]+1;
                                                  while((!fixed)&&(b9 = obj->Neighbor[n8])>=0) {
                                                    if(b9==b0) {  /* 7-cycle */
                                                      fixed=true;
                                                    } else if((b9!=b7)&&(b9!=b2)) {
                                                      n9 = obj->Neighbor[b9]+1;
                                                      while((!fixed)&&(b10 = obj->Neighbor[n9])>=0) {
                                                        if(b10==b0) {  /* 8-cycle */
                                                          fixed=true;
                                                        } 
                                                        n9+=2;
                                                      }
                                                    }
                                                    n8+=2;
                                                  }
                                                }
                                                n7+=2;
                                              }
                                            }
                                            n6+=2;
                                          }
                                        }
                                        n5+=2;
                                      }
                                    }
                                    n4+=2;
                                  }
                                }
                                n3+=2;
                              }
                            }
#endif
                            /* don't get jacked by pseudo-planar PRO */

                            if(((oai[b0].protons!=cAN_N)||
                                (!WordMatchExact(G,oai[b0].resn,"PRO",true)))&&
                               ((oai[b2].protons!=cAN_N)||
                                (!WordMatchExact(G,oai[b2].resn,"PRO",true)))) {
                                     
                              if(use_cache) {
                                if(!SculptCacheQuery(G,cSculptPlan,
                                                     oai[b1].unique_id,
                                                     oai[b0].unique_id,
                                                     oai[b2].unique_id,
                                                     oai[b3].unique_id,
                                                     &d))
                                  SculptCacheStore(G,cSculptPlan,
                                                   oai[b1].unique_id,
                                                   oai[b0].unique_id,
                                                   oai[b2].unique_id,
                                                   oai[b3].unique_id,
                                                   d);
                              }
                              ShakerAddPlanCon(I->Shaker,b1,b0,b2,b3,d,fixed); 
                            }
                          }
                        }
                      }
                    }
                    n2+=2;
                  }
                }
                n1+=2;
              }
              n0+=2;
            }
          }

          {
          /* longer-range exclusions (1-5,1-6,1-7,1-8,1-9) -- only locate & store when needed */
            
            int mask = SettingGet_i(G,cs->Setting,obj->Obj.Setting,cSetting_sculpt_field_mask);
            int max_excl = SettingGet_i(G,cs->Setting,obj->Obj.Setting,cSetting_sculpt_avd_excl);
            if(max_excl>9)
              max_excl = 9;

            if((cSculptAvoid & mask)&&(max_excl>4)) {
              int b_stack[10];
              int n_stack[10];
              int stop_depth = max_excl-1;
              int depth;
              int bd,skip;
              for(b0=0;b0<obj->NAtom;b0++) {
                b_stack[0] = b0;
                n_stack[0] = obj->Neighbor[b_stack[0]]+1;
                depth = 0;
                while(depth>=0) {
                  if((bd = obj->Neighbor[n_stack[depth]])<0) {
                    depth--;
                    if(depth>=0) { /* iterate next atom */
                      n_stack[depth] += 2;
                    }
                  } else {
                    skip = (depth==stop_depth);
                    if(!skip) {
                      for(a=0;a<depth;a++) {
                        if(b_stack[a]==bd) {
                          skip=true;
                          break;
                        }
                      }
                    }
                    if(!skip) {
                      depth++;
                      b_stack[depth] = bd;
                      n_stack[depth] = obj->Neighbor[bd] + 1;
                      if((depth>3)&&(b0<bd)) {

                        xhash = ex_hash(b0,bd);
                      
                        VLACheck(I->EXList,int,nex+3);
                        j = I->EXList+nex;
                        *(j++)=*(I->EXHash+xhash);
                        *(j++)=b0;
                        *(j++)=bd;
                        *(j++)= depth+1; /* 1-5, 1-6, 1-7 etc. */
                        *(I->EXHash+xhash)=nex;
                        nex+=4;
                      }
                    } else {
                      n_stack[depth] += 2;
                    }
                  }
                }
              }
            }
          }
          FreeP(planer);
          FreeP(linear);
          FreeP(single);
        }
      }
  
  PRINTFB(G,FB_Sculpt,FB_Blather)
    " Sculpt: I->Shaker->NDistCon %d\n",I->Shaker->NDistCon
    ENDFB(G);
  PRINTFB(G,FB_Sculpt,FB_Blather)
    " Sculpt: I->Shaker->NPyraCon %d\n",I->Shaker->NPyraCon
    ENDFB(G);
  PRINTFB(G,FB_Sculpt,FB_Blather)
    " Sculpt: I->Shaker->NPlanCon %d\n",I->Shaker->NPlanCon
    ENDFB(G);
  
  
  PRINTFD(G,FB_Sculpt)
    " SculptMeasureObject-Debug: leaving...\n"
    ENDFD;
  
}
#ifdef _PYMOL_INLINE
__inline__
#endif
static int SculptCheckBump(float *v1,float *v2,float *diff,
                           float *dist,float cutoff)
{
  register float d2;
  diff[0] = (v1[0]-v2[0]);
  diff[1] = (v1[1]-v2[1]);
  if(fabs(diff[0])>cutoff) return(false);
  diff[2] = (v1[2]-v2[2]);
  if(fabs(diff[1])>cutoff) return(false);
  if(fabs(diff[2])>cutoff) return(false);
  d2 = (diff[0]*diff[0] + diff[1]*diff[1] + diff[2]*diff[2]);
  if(d2<(cutoff*cutoff)) {
    *dist = (float)sqrt(d2);
    return(true);
  }
  return(false);
}

#ifdef _PYMOL_INLINE
__inline__
#endif
static int SculptCGOBump(float *v1,float *v2,
                         float vdw1, float vdw2, 
                         float cutoff, 
                         float min,float mid, float max,
                         float *good_color, 
                         float *bad_color,
                         int mode,
                         CGO *cgo)
{
  register float d2;
  float diff[3];
  register float dist;
  register float min_cutoff = cutoff - min;

  diff[0] = (v1[0]-v2[0]);
  diff[1] = (v1[1]-v2[1]);
  if(fabs(diff[0])>min_cutoff) return(false);
  diff[2] = (v1[2]-v2[2]);
  if(fabs(diff[1])>min_cutoff) return(false);
  if(fabs(diff[2])>min_cutoff) return(false);
  d2 = (diff[0]*diff[0] + diff[1]*diff[1] + diff[2]*diff[2]);
  if(d2>(min_cutoff*min_cutoff)) {
    return false;
  } else {
    dist = (float)sqrt(d2);
    if(dist<=min_cutoff) {
      float avg[3],vv1[3],vv2[3],tmp[3],color[3];
      float good_bad = cutoff - dist; /* if negative, then good */
      float color_factor;
      float radius = 0.5*(good_bad - min);
      
      if(good_bad<mid) {
        color_factor = 0.0F;
      } else {
        color_factor = (good_bad-mid)/max;
        if(color_factor>1.0F)
          color_factor = 1.0F;
      }

      {
        float one_minus_color_factor = 1.0F - color_factor;
        scale3f(bad_color,color_factor,color);
        scale3f(good_color, one_minus_color_factor,tmp);
        add3f(tmp,color,color);

        switch(mode) {
        case 2:
          if(good_bad>mid) {
            CGOLinewidth(cgo,1+color_factor*3);
            CGOColorv(cgo,color);
            CGOBegin(cgo,GL_LINES);
            CGOVertexv(cgo,v1);
            CGOVertexv(cgo,v2);
            CGOEnd(cgo);
          }
          break;
        case 1:
          {
            float delta,one_minus_delta;
            if(good_bad<0.0) {
              delta = fabs(good_bad);
            } else {
              delta = 0.5*(0.01F + fabs(good_bad))/(cutoff);
            }
            if(delta<0.01F) delta=0.01F;
            if(delta>0.1F) {
              delta = 0.1F;
            }
            if(radius<0.01F) radius=0.01F;

            one_minus_delta = 1.0F - delta;
            scale3f(v2,vdw1,avg);
            scale3f(v1,vdw2,tmp);
            add3f(tmp,avg,avg);
            {
              register float inv = 1.0F/(vdw1+vdw2);
              scale3f(avg,inv,avg);
            }
            scale3f(v1,delta,vv1);
            scale3f(avg,one_minus_delta,tmp);
            add3f(tmp,vv1,vv1);
            scale3f(v2,delta,vv2);
            scale3f(avg,one_minus_delta,tmp);
            add3f(tmp,vv2,vv2);
            if(good_bad<0.0F) {
              CGOLinewidth(cgo,1+color_factor*3);
              CGOResetNormal(cgo,true);
              CGOColorv(cgo,color);
              CGOBegin(cgo,GL_LINES);
              CGOVertexv(cgo,vv1);
              CGOVertexv(cgo,vv2);
              CGOEnd(cgo);
            } else {
              CGOCustomCylinderv(cgo, vv1, vv2, radius,
                                 color,color,
                                 1,1);
            }
          }
          break;
        }
      }
    }
    if(dist>cutoff)
      return false;
    return(true);
  }
}

#ifdef _PYMOL_INLINE
__inline__
#endif
static int SculptDoBump(float target,float actual,float *d,
                 float *d0to1,float *d1to0,float wt,float *strain)
{
  float push[3];
  float dev,dev_2,sc,abs_dev;

  dev = target-actual;
  if((abs_dev=(float)fabs(dev))>R_SMALL8) {
    dev_2 = wt*dev/2.0F;
    (*strain) += abs_dev;
    if(actual>R_SMALL8) { /* nonoverlapping */
      sc = dev_2/actual;
      scale3f(d,sc,push);
      add3f(push,d0to1,d0to1);
      subtract3f(d1to0,push,d1to0);
    } else { /* overlapping, so just push along X */
      d0to1[0]-=dev_2;
      d1to0[0]+=dev_2;
    }
    return 1;
  }
  return 0;
}

#ifdef _PYMOL_INLINE
__inline__
#endif
static int SculptCheckAvoid(float *v1,float *v2,float *diff,
                            float *dist, float avoid,float range)
{
  register float d2,l2;
  register float cutoff = avoid+range;
  register float low_cutoff;
  diff[0] = (v1[0]-v2[0]);
  diff[1] = (v1[1]-v2[1]);
  if(fabs(diff[0])>cutoff) return(false);
  diff[2] = (v1[2]-v2[2]);
  if(fabs(diff[1])>cutoff) return(false);
  low_cutoff = avoid-range;
  if(fabs(diff[2])>cutoff) return(false);
  l2 = low_cutoff * low_cutoff;
  d2 = (diff[0]*diff[0] + diff[1]*diff[1] + diff[2]*diff[2]);
  if((d2<(cutoff*cutoff))&&(d2>l2)) { /* we are in the avoid range */
    *dist = (float)sqrt(d2);
    return(true);
  }
  return(false);
}

#ifdef _PYMOL_INLINE
__inline__
#endif
static int SculptDoAvoid(float avoid, float range, float actual,float *d,
                 float *d0to1,float *d1to0,float wt,float *strain)
{
  float push[3];
  float dev,dev_2,sc,abs_dev;
  float target;
  if(actual>avoid) {
    target = avoid+range;
  } else {
    target = avoid-range;
  }
  dev = target-actual;
  if((abs_dev=(float)fabs(dev))>R_SMALL8) {
    dev_2 = wt*dev/2.0F;
    (*strain) += abs_dev;
    if(actual>R_SMALL8) { /* nonoverlapping */
      sc = dev_2/actual;
      scale3f(d,sc,push);
      add3f(push,d0to1,d0to1);
      subtract3f(d1to0,push,d1to0);
    } else { /* overlapping, so just push along X */
      d0to1[0]-=dev_2;
      d1to0[0]+=dev_2;
    }
    return 1;
  }
  return 0;
}

float SculptIterateObject(CSculpt *I,ObjectMolecule *obj,
                          int state,int n_cycle,float *center)
{
  PyMOLGlobals *G=I->G;
  register CShaker *shk;
  register int a,a0,a1,a2,a3,b0,b1,b2,b3;
  int aa;
  CoordSet *cs;
  float *disp = NULL;
  float *v,*v0,*v1,*v2,*v3;
  float diff[3],len;
  int *atm2idx = NULL;
  int *cnt = NULL;
  int *i;
  int hash;
  int nb_next;
  int h,k,l;
  int offset;
  float cutoff,vdw_cutoff;
  int ex;
  int eval_flag;
  int mask;
  float wt;
  float vdw;
  float vdw14;
  float vdw_wt;
  float vdw_wt14;
  float bond_wt;
  float angl_wt;
  float pyra_wt;
  float plan_wt;
  float line_wt;
  float tors_wt;
  float tors_tole;
  int active_flag=false;
  float hb_overlap,hb_overlap_base;
  int *active,n_active;
  AtomInfoType *ai0,*ai1;
  double task_time;
  float vdw_magnify,vdw_magnified = 1.0F;
  int nb_skip,nb_skip_count;
  float total_strain=0.0F,strain;
  int total_count=1;
  CGO *cgo = NULL;
  float good_color[3] = { 0.2, 1.0, 0.2};
  float bad_color[3] = { 1.0, 0.2, 0.2};
  int vdw_vis_mode;
  float vdw_vis_min=0.0F,vdw_vis_mid=0.0F,vdw_vis_max=0.0F;
  float tri_sc, tri_wt;
  float min_sc, min_wt;
  float max_sc = 1.025F, max_wt=0.75F;
  float *cs_coord;
  float solvent_radius;
  float avd_wt, avd_gp, avd_rg;
  int avd_ex;

  PRINTFD(G,FB_Sculpt)
    " SculptIterateObject-Debug: entered state=%d n_cycle=%d\n",state,n_cycle
    ENDFD;
  if(!n_cycle)
    n_cycle=-1;

  if((state<obj->NCSet) && obj->CSet[state] && n_cycle) {
    
    disp = Alloc(float,3*obj->NAtom);
    atm2idx = Alloc(int,obj->NAtom);
    cnt = Alloc(int,obj->NAtom);
    active = Alloc(int,obj->NAtom);
    shk=I->Shaker;

    PRINTFD(G,FB_Sculpt)
      " SIO-Debug: NDistCon %d\n",shk->NDistCon
      ENDFD;

    cs = obj->CSet[state];
    cs_coord = cs->Coord;

    vdw = SettingGet_f(G,cs->Setting,obj->Obj.Setting,cSetting_sculpt_vdw_scale);
    vdw14 = SettingGet_f(G,cs->Setting,obj->Obj.Setting,cSetting_sculpt_vdw_scale14);
    vdw_wt = SettingGet_f(G,cs->Setting,obj->Obj.Setting,cSetting_sculpt_vdw_weight);
    vdw_wt14 = SettingGet_f(G,cs->Setting,obj->Obj.Setting,cSetting_sculpt_vdw_weight14);
    bond_wt =  SettingGet_f(G,cs->Setting,obj->Obj.Setting,cSetting_sculpt_bond_weight);
    angl_wt =  SettingGet_f(G,cs->Setting,obj->Obj.Setting,cSetting_sculpt_angl_weight);
    pyra_wt =  SettingGet_f(G,cs->Setting,obj->Obj.Setting,cSetting_sculpt_pyra_weight);
    plan_wt =  SettingGet_f(G,cs->Setting,obj->Obj.Setting,cSetting_sculpt_plan_weight);
    line_wt =  SettingGet_f(G,cs->Setting,obj->Obj.Setting,cSetting_sculpt_line_weight);
    tri_wt =  SettingGet_f(G,cs->Setting,obj->Obj.Setting,cSetting_sculpt_tri_weight);
    tri_sc =  SettingGet_f(G,cs->Setting,obj->Obj.Setting,cSetting_sculpt_tri_scale);


    min_wt =  SettingGet_f(G,cs->Setting,obj->Obj.Setting,cSetting_sculpt_min_weight);
    min_sc =  SettingGet_f(G,cs->Setting,obj->Obj.Setting,cSetting_sculpt_min_scale);
    max_wt =  SettingGet_f(G,cs->Setting,obj->Obj.Setting,cSetting_sculpt_max_weight);
    max_sc =  SettingGet_f(G,cs->Setting,obj->Obj.Setting,cSetting_sculpt_max_scale);

    mask = SettingGet_i(G,cs->Setting,obj->Obj.Setting,cSetting_sculpt_field_mask);
    hb_overlap = SettingGet_f(G,cs->Setting,obj->Obj.Setting,cSetting_sculpt_hb_overlap);
    hb_overlap_base = SettingGet_f(G,cs->Setting,obj->Obj.Setting,cSetting_sculpt_hb_overlap_base);
    tors_tole = SettingGet_f(G,cs->Setting,obj->Obj.Setting,cSetting_sculpt_tors_tolerance);
    tors_wt = SettingGet_f(G,cs->Setting,obj->Obj.Setting,cSetting_sculpt_tors_weight);
    vdw_vis_mode = SettingGet_i(G,cs->Setting,obj->Obj.Setting,cSetting_sculpt_vdw_vis_mode);
    solvent_radius = SettingGet_f(G,cs->Setting,obj->Obj.Setting,cSetting_solvent_radius);

    avd_wt =  SettingGet_f(G,cs->Setting,obj->Obj.Setting,cSetting_sculpt_avd_weight);
    avd_gp = SettingGet_f(G,cs->Setting,obj->Obj.Setting,cSetting_sculpt_avd_gap);
    avd_rg = SettingGet_f(G,cs->Setting,obj->Obj.Setting,cSetting_sculpt_avd_range);
    avd_ex = SettingGet_f(G,cs->Setting,obj->Obj.Setting,cSetting_sculpt_avd_excl);
    if(avd_gp<0.0F)
      avd_gp = 1.5F*solvent_radius;
    if(avd_rg<0.0F)
      avd_rg = solvent_radius;
    
    if(vdw_vis_mode) {
      vdw_vis_min =  SettingGet_f(G,cs->Setting,obj->Obj.Setting,cSetting_sculpt_vdw_vis_min);
      vdw_vis_mid =  SettingGet_f(G,cs->Setting,obj->Obj.Setting,cSetting_sculpt_vdw_vis_mid);
      vdw_vis_max =  SettingGet_f(G,cs->Setting,obj->Obj.Setting,cSetting_sculpt_vdw_vis_max);

      if(!cs->SculptCGO)
        cs->SculptCGO = CGONew(G);
      else
        CGOReset(cs->SculptCGO);
      cgo = cs->SculptCGO;
    } else if(cs->SculptCGO) {
      CGOReset(cs->SculptCGO);      
    }

    nb_skip = SettingGet_i(G,cs->Setting,obj->Obj.Setting,cSetting_sculpt_nb_interval);
    if(nb_skip>n_cycle)
      nb_skip = n_cycle;
    if(nb_skip<0) nb_skip=0;

    n_active = 0;
    ai0=obj->AtomInfo;
    for(a=0;a<obj->NAtom;a++) {
      if(ai0->flags&cAtomFlag_exclude) {
        a1=-1;
      } else {
        if(obj->DiscreteFlag) {
          if(cs==obj->DiscreteCSet[a]) {
            a1=obj->DiscreteAtmToIdx[a];
          } else {
            a1=-1;
          }
        } else {
          a1=cs->AtmToIdx[a];
        }
      }
      if(a1>=0) {
        active_flag=true;
        active[n_active] = a;
        n_active++;
      }
      atm2idx[a] = a1;
      ai0++;
    }
        
    if(active_flag) {

      /* first, create coordinate -> vertex mapping */
      /* and count number of constraints */

      task_time = UtilGetSeconds(G);
      vdw_magnify = 1.0F;
      nb_skip_count = 0;

      if(center) {
        int *a_ptr = active;
        for(aa=0;aa<n_active;aa++) {
          a = *(a_ptr++);
          if(obj->AtomInfo[a].protekted!=1) {
            v2 = cs_coord+3*atm2idx[a];
            center[4] += *(v2  );
            center[5] += *(v2+1);
            center[6] += *(v2+2);
            center[7] += 1.0F;
          }
        }
      }

      while(n_cycle--) {
            
        total_strain = 0.0F;
        total_count = 0;
        /* initialize displacements to zero */
        
        v = disp;
        i = cnt;
        for(aa=0;aa<n_active;aa++) {
          a = active[aa];
          v=disp+a*3;
          cnt[a]=0;
          *(v  )=0.0F;
          *(v+1)=0.0F;          
          *(v+2)=0.0F;
        }
          
        /* apply distance constraints */

        {
          register ShakerDistCon *sdc=shk->DistCon;
          int ndc = shk->NDistCon;
          for(a=0;a<ndc;a++) {
            b1 = sdc->at0;
            b2 = sdc->at1;

            switch(sdc->type) {
            case cShakerDistBond:
              eval_flag = cSculptBond & mask;
              wt = bond_wt;
              break;
            case cShakerDistAngle:
              eval_flag = cSculptAngl & mask;
              wt = angl_wt;
              break;
            case cShakerDistLimit:
              eval_flag = cSculptTri & mask;
              wt = tri_wt;
              break;
            case cShakerDistMinim:
              eval_flag = cSculptMin & mask;
              wt = min_wt * sdc->weight;
              break;
            case cShakerDistMaxim:
              eval_flag = cSculptMax & mask;
              wt = max_wt * sdc->weight;
              break;
            default:
              eval_flag = false;
              wt=0.0F;
              break;
            }
            
            if(eval_flag) {
              a1 = atm2idx[b1]; /* coordinate set indices */
              a2 = atm2idx[b2];
              if((a1>=0)&&(a2>=0)) {
                v1 = cs_coord+3*a1;
                v2 = cs_coord+3*a2;
                switch(sdc->type) {
                case cShakerDistLimit:
                  strain = ShakerDoDistLimit(sdc->targ*tri_sc,v1,v2,disp+b1*3,disp+b2*3,wt);
                  if(strain>0.0F) {
                    cnt[b1]++;
                    cnt[b2]++;
                    total_strain+=strain;
                    total_count++;
                  }
                  break;
                case cShakerDistMaxim:
                  strain = ShakerDoDistLimit(sdc->targ*max_sc,v1,v2,disp+b1*3,disp+b2*3,wt);
                  if(strain>0.0F) {
                    cnt[b1]++;
                    cnt[b2]++;
                    total_strain+=strain;
                    total_count++;
                  }
                  break;
                case cShakerDistMinim:
                  strain = ShakerDoDistMinim(sdc->targ*min_sc,v1,v2,disp+b1*3,disp+b2*3,wt);
                  if(strain>0.0F) {
                    cnt[b1]++;
                    cnt[b2]++;
                    total_strain+=strain;
                    total_count++;
                  }
                  break;
                default:
                  total_strain+=ShakerDoDist(sdc->targ,v1,v2,disp+b1*3,disp+b2*3,wt);
                  cnt[b1]++;
                  cnt[b2]++;
                  total_count++;
                }
              }
            }
            sdc++;
          }
        }
        /* apply line constraints */
      
      if(cSculptLine & mask) {
	  register ShakerLineCon *slc=shk->LineCon;
	  int nlc = shk->NLineCon;
          for(a=0;a<nlc;a++) {
            b0 = slc->at0;
            b1 = slc->at1;
            b2 = slc->at2;
            a0 = atm2idx[b0]; /* coordinate set indices */
            a1 = atm2idx[b1];
            a2 = atm2idx[b2];
                
            if((a0>=0)&&(a1>=0)&&(a2>=0))
              {
                cnt[b0]++;
                cnt[b1]++;
                cnt[b2]++;
                v0 = cs_coord+3*a0;
                v1 = cs_coord+3*a1;
                v2 = cs_coord+3*a2;
                total_strain+=ShakerDoLine(v0,v1,v2,disp+b0*3,disp+b1*3,disp+b2*3,line_wt);
                total_count++;
              }
            slc++;
          }
        }

        /* apply pyramid constraints */

        if(cSculptPyra & mask) {
	  register ShakerPyraCon *spc=shk->PyraCon;
	  int npc = shk->NPyraCon;
          for(a=0;a<npc;a++) {
              
            b0 = spc->at0;
            b1 = spc->at1;
            b2 = spc->at2;
            b3 = spc->at3;
            a0 = atm2idx[b0];
            a1 = atm2idx[b1];
            a2 = atm2idx[b2];
            a3 = atm2idx[b3];
              
            if((a0>=0)&&(a1>=0)&&(a2>=0)&&(a3>=0)) {
              v0 = cs_coord+3*a0;
              v1 = cs_coord+3*a1;
              v2 = cs_coord+3*a2;
              v3 = cs_coord+3*a3;
              total_strain+=ShakerDoPyra(spc->targ,
                                         v0,v1,v2,v3,
                                         disp+b0*3,
                                         disp+b1*3,
                                         disp+b2*3,
                                         disp+b3*3,
                                         pyra_wt);
              total_count++;
                
              cnt[b0]++;
              cnt[b1]++;
              cnt[b2]++;
              cnt[b3]++;
            }
            spc++;
          }
        }

        if(cSculptPlan & mask) {
	  register ShakerPlanCon *snc = shk->PlanCon;
	  int npc = shk->NPlanCon;
          /* apply planarity constraints */
          
          snc=shk->PlanCon;
          for(a=0;a<npc;a++) {
              
            b0 = snc->at0;
            b1 = snc->at1;
            b2 = snc->at2;
            b3 = snc->at3;
            a0 = atm2idx[b0];
            a1 = atm2idx[b1];
            a2 = atm2idx[b2];
            a3 = atm2idx[b3];
              
            if((a0>=0)&&(a1>=0)&&(a2>=0)&&(a3>=0)) {
              v0 = cs_coord+3*a0;
              v1 = cs_coord+3*a1;
              v2 = cs_coord+3*a2;
              v3 = cs_coord+3*a3;
              total_strain+=ShakerDoPlan(v0,v1,v2,v3,
                                         disp+b0*3,
                                         disp+b1*3,
                                         disp+b2*3,
                                         disp+b3*3,
                                         snc->target,
                                         snc->fixed,
                                         plan_wt);
              total_count++;
              cnt[b0]++;
              cnt[b1]++;
              cnt[b2]++;
              cnt[b3]++;
            }
              
            snc++;
          }
        }
            
        /* apply torsion constraints */

        if(cSculptTors & mask) {
	  register ShakerTorsCon *stc = shk->TorsCon;
	  int ntc = shk->NTorsCon;
          /* apply planarity constraints */

          for(a=0;a<ntc;a++) {
              
            b0 = stc->at0;
            b1 = stc->at1;
            b2 = stc->at2;
            b3 = stc->at3;
            a0 = atm2idx[b0];
            a1 = atm2idx[b1];
            a2 = atm2idx[b2];
            a3 = atm2idx[b3];
              
            if((a0>=0)&&(a1>=0)&&(a2>=0)&&(a3>=0)) {
              v0 = cs_coord+3*a0;
              v1 = cs_coord+3*a1;
              v2 = cs_coord+3*a2;
              v3 = cs_coord+3*a3;
              total_strain+=ShakerDoTors(stc->type,
                                         v0,v1,v2,v3,
                                         disp+b0*3,
                                         disp+b1*3,
                                         disp+b2*3,
                                         disp+b3*3,
                                         tors_tole,
                                         tors_wt);
              total_count++;
              cnt[b0]++;
              cnt[b1]++;
              cnt[b2]++;
              cnt[b3]++;
            }
            stc++;
          }
        }
        /* apply nonbonded interactions */

        if((n_cycle>0) && 
           (nb_skip_count>0)) { 
          /*skip and then weight extra */
          nb_skip_count--;
          vdw_magnify += 1.0F;
        } else {
          int nb_off0,nb_off1;
          int v0i,v1i,v2i;
          int x0i;
          int don_b0;
          int acc_b0;
          
          vdw_magnified = vdw_magnify;
          vdw_magnify = 1.0F;

          nb_skip_count = nb_skip;
          if((cSculptVDW|cSculptVDW14|cSculptAvoid)&mask) {
            /* compute non-bonded interations */
                
            /* construct nonbonded hash */
                
            nb_next = 1;
            for(aa=0;aa<n_active;aa++) {
              b0 = active[aa];
              a0 = atm2idx[b0];
              VLACheck(I->NBList,int,nb_next+2);
              v0 = cs_coord+3*a0;
              hash = nb_hash(v0);
              i = I->NBList+nb_next;
              *(i++)=*(I->NBHash+hash);
              *(i++)=hash;
              *(i++)=b0;
              *(I->NBHash+hash)=nb_next;
              nb_next+=3;
            }
                
            /* find neighbors for each atom */
            if((cSculptVDW|cSculptVDW14)&mask) {
              for(aa=0;aa<n_active;aa++) {
                b0 = active[aa];
                a0 = atm2idx[b0];
                ai0=obj->AtomInfo+b0;
                v0 = cs_coord+3*a0;
                don_b0 = I->Don[b0];
                acc_b0 = I->Acc[b0];
                v0i = (int)(*v0);
                v1i = (int)(*(v0+1));
                v2i = (int)(*(v0+2));
                x0i = ex_hash_i0(b0);
                for(h=-4;h<5;h+=4) {
                  nb_off0 = nb_hash_off_i0(v0i,h);
                  for(k=-4;k<5;k+=4) {
                    nb_off1 = nb_off0 | nb_hash_off_i1(v1i,k);
                    for(l=-4;l<5;l+=4) { 
                      /*  offset = *(I->NBHash+nb_hash_off(v0,h,k,l));*/
                      offset = *(I->NBHash + (nb_off1 | nb_hash_off_i2(v2i,l)));
                      while(offset) {
                        i = I->NBList + offset;
                        b1 = *(i+2);
                        if(b1>b0) { 
                          /* determine exclusion (if any) */
                          {
                            register int xoffset;
                            register int *I_EXList = I->EXList;
                            register int ex1;
                            register int *j;
                            xoffset = *(I->EXHash+ (x0i | ex_hash_i1(b1)));
                            ex = 10;
                            while(xoffset) {
                              xoffset = (*(j = I_EXList + xoffset));
                              if((*(j+1)==b0)&&(*(j+2)==b1)) {
                                ex1 = *(j+3);
                                if(ex1<ex) {
                                  ex=ex1;
                                }
                              }
                            }
                          }
                          if(ex>3) {
                            ai1=obj->AtomInfo+b1;
                            cutoff = ai0->vdw+ai1->vdw;
                          
                            if(ex==4) { /* 1-4 interation */
                              cutoff*=vdw14;
                              wt = vdw_wt14 * vdw_magnified;
                            
                              if(cSculptVDW14 & mask) {
                                a1 = atm2idx[b1];
                                v1 = cs_coord+3*a1;
                                if(SculptCheckBump(v0,v1,diff,&len,cutoff)) {
                                  if(SculptDoBump(cutoff,len,diff,
                                                  disp+b0*3,disp+b1*3,wt,&total_strain)) {
                                    cnt[b0]++;
                                    cnt[b1]++;
                                    total_count++;
                                  }
                                }
                              }
                            } else { /* standard interaction */
                              if(don_b0&&I->Acc[b1]) { /* h-bond */
                                if(ai0->protons==cAN_H) {
                                  cutoff-=hb_overlap;
                                } else {
                                  cutoff-=hb_overlap_base;
                                }
                              } else if(acc_b0&&I->Don[b1]) { /* h-bond */
                                if(ai1->protons==cAN_H) {
                                  cutoff-=hb_overlap;
                                } else {
                                  cutoff-=hb_overlap_base;
                                } 
                              }
                              if(cSculptVDW & mask) {
                                vdw_cutoff=cutoff*vdw;
                                wt = vdw_wt * vdw_magnified;
                                a1 = atm2idx[b1];
                                v1 = cs_coord+3*a1;
                                if(vdw_vis_mode && cgo && 
                                   (n_cycle<1) && (!(ai0->protekted&&ai1->protekted))) {
                                  SculptCGOBump(v0,v1,ai0->vdw,ai1->vdw,
                                                cutoff,vdw_vis_min,
                                                vdw_vis_mid, vdw_vis_max, 
                                                good_color, bad_color, 
                                                vdw_vis_mode,cgo);
                                }
                                if(SculptCheckBump(v0,v1,diff,&len,vdw_cutoff))
                                  if(SculptDoBump(vdw_cutoff,len,diff,
                                                  disp+b0*3,disp+b1*3,wt,&total_strain)) {
                                    cnt[b0]++;
                                    cnt[b1]++;
                                    total_count++;
                                  }
                              }
                            }
                          }
                        }
                        offset=(*i);
                      }
                    }
                  }
                }
              }
            }

            if(cSculptAvoid&mask) {
              float target;
              float range = solvent_radius*0.75;
              /* tweak nb distances to avoid
                 sitting in the surface
                 rendition danger zone for too
                 long (vdw1+vdw2+0.75*solvent) */
              for(aa=0;aa<n_active;aa++) {
                b0 = active[aa];
                a0 = atm2idx[b0];
                ai0=obj->AtomInfo+b0;
                v0 = cs_coord+3*a0;
                don_b0 = I->Don[b0];
                acc_b0 = I->Acc[b0];
                v0i = (int)(*v0);
                v1i = (int)(*(v0+1));
                v2i = (int)(*(v0+2));
                x0i = ex_hash_i0(b0);
                for(h=-8;h<9;h+=4) {
                  nb_off0 = nb_hash_off_i0(v0i,h);
                  for(k=-8;k<9;k+=4) {
                    nb_off1 = nb_off0 | nb_hash_off_i1(v1i,k);
                    for(l=-8;l<9;l+=4) {
                      /*  offset = *(I->NBHash+nb_hash_off(v0,h,k,l));*/
                      offset = *(I->NBHash + (nb_off1 | nb_hash_off_i2(v2i,l)));
                      while(offset) {
                        i = I->NBList + offset;
                        b1 = *(i+2);
                        if(b1>b0) { 
                          /* determine exclusion (if any) */
                          {
                            register int xoffset;
                            register int *I_EXList = I->EXList;
                            register int ex1;
                            register int *j;
                            xoffset = *(I->EXHash+ (x0i | ex_hash_i1(b1)));
                            ex = 10;
                            while(xoffset) {
                              xoffset = (*(j = I_EXList + xoffset));
                              if((*(j+1)==b0)&&(*(j+2)==b1)) {
                                ex1 = *(j+3);
                                if(ex1<ex) {
                                  ex=ex1;
                                }
                              }
                            }
                          }
                          if(ex>avd_ex) {  /* either non-covalent or extended chain */
                            ai1=obj->AtomInfo+b1;
                            target = ai0->vdw+ai1->vdw+avd_gp;
                            a1 = atm2idx[b1];
                            v1 = cs_coord+3*a1;
                            
                            if(SculptCheckAvoid(v0,v1,diff,&len,target,avd_rg)) {
                              if(SculptDoAvoid(target,range,len,diff,
                                              disp+b0*3,disp+b1*3,avd_wt,&total_strain)) {
                                cnt[b0]++;
                                cnt[b1]++;
                                total_count++;
                              }
                            }
                          }
                        }
                        offset=(*i);
                      }
                    }
                  }
                }
              }
            }
                
            /* clean up nonbonded hash */
                
            i = I->NBList+2;
            while(nb_next>1) {
              *(I->NBHash+*i)=0; 
              i+=3;
              nb_next-=3;
            }
          }
        }
        /* average the displacements */
             
        if(n_cycle>=0) {
          int cnt_a;
          float _1 = 1.0F;
          register float inv_cnt;
          int *a_ptr = active;
          register float *lookup_inverse = I->inverse;
          for(aa=0;aa<n_active;aa++) {
            if( (cnt_a = cnt[(a = *(a_ptr++))]) ) {
              if(!obj->AtomInfo[a].protekted) {
                v1 = disp+3*a;
                v2 = cs_coord+3*atm2idx[a];
                if(!(cnt_a&0xFFFFFF00)) /* don't divide -- too slow */
                  inv_cnt = lookup_inverse[cnt_a];
                else
                  inv_cnt = _1/cnt_a;
                *(v2  )+=(*(v1  ))*inv_cnt;
                *(v2+1)+=(*(v1+1))*inv_cnt;
                *(v2+2)+=(*(v1+2))*inv_cnt;
              }
            }
          }
          if(cs->fInvalidateRep) {
            cs->fInvalidateRep(cs,cRepAll,cRepInvCoord);
          } else {
            ObjectMoleculeInvalidate(obj,cRepAll,cRepInvCoord,-1);
          }
        } else if(cgo) {
          SceneDirty(G);
        }
        if(n_cycle<=0) {
          int *a_ptr = active;
          if(center) 
           for(aa=0;aa<n_active;aa++) {
             a = *(a_ptr++);
             if(obj->AtomInfo[a].protekted!=1) {
               v2 = cs_coord+3*atm2idx[a];
               center[0] += *(v2  );
               center[1] += *(v2+1);
               center[2] += *(v2+2);
               center[3] += 1.0F;
             }
           }
          break;
        }
      }
          
      task_time = UtilGetSeconds(G) - task_time;
      PRINTFB(G,FB_Sculpt,FB_Blather)
        " Sculpt: %2.5f seconds %8.3f %d %8.3f\n",task_time,total_strain,total_count,
        100*total_strain/total_count
        ENDFB(G);

      if(total_count) 
        total_strain = (1000*total_strain)/total_count;
    }
    FreeP(active);
    FreeP(cnt);
    FreeP(disp);
    FreeP(atm2idx);
    if(cgo) {
      CGOStop(cgo);
      {
        int est=CGOCheckComplex(cgo);
        if(est) {
          cs->SculptCGO = CGOSimplify(cgo,est);
          CGOFree(cgo);
        }
      }
    }
  }

  PRINTFD(G,FB_Sculpt)
    " SculptIterateObject-Debug: leaving...\n"
    ENDFD;

  return total_strain;
}

void SculptFree(CSculpt *I)
{
  VLAFreeP(I->Don);
  VLAFreeP(I->Acc);
  VLAFreeP(I->NBList);
  VLAFreeP(I->EXList);

  FreeP(I->NBHash);
  FreeP(I->EXHash);
  ShakerFree(I->Shaker);
  OOFreeP(I);
}

#if 0
 

                                    if(ex1==4) { /* special handling for 1-4's */
                                      ex14_b0=*(j+4); /* b1\b0_b2/b3 atoms */
                                      ex14_b2=*(j+5);
                                    }

                                      if(SculptDoBump14(cutoff,len,diff,
                                                        disp+b0*3,disp+b1*3,
                                                        wt,&total_strain,
                                                        v0,
                                                        v1,
                                                        cs->Coord+3*atm2idx[ex14_b0],
                                                        cs->Coord+3*atm2idx[ex14_b2],
                                                        disp+ex14_b0*3,
                                                        disp+ex14_b2*3)) {
                                        cnt[b0]++;
                                        cnt[b1]++;
                                        total_count++;
                                      }
                                  }

#ifdef _PYMOL_INLINE
__inline__
#endif
static int SculptDoBump14(float target,float actual,float *d,
                          float *d0to1,float *d1to0,float wt,float *strain,
                          float *v0,float *v1,float *v2,float *v3,
                          float *d2to3,float *d3to2)
{
  /* v0\v2_v3/v1 */

  float push0[3],push1[3],balance[3];
  float axis[3],seg0[3],seg1[3],perp0[3],perp1[3];
  float dir[3];
  float dev,dev_2,sc,abs_dev;
  float sign,dp;

  dev = target-actual;
  if((abs_dev=fabs(dev))>R_SMALL8) {

    /* v0        v1
         \      /
         v2__v3 */

    subtract3f(v3,v2,axis);    
    subtract3f(v0,v2,seg0);
    normalize3f(axis);
    subtract3f(v1,v3,seg1);
    cross_product3f(seg0,axis,perp0);
    cross_product3f(axis,seg1,perp1);

    normalize3f(perp0);
    normalize3f(perp1);

#if 0
    {
      float vert[3];
    /* debuggin */

      CGOColor(DebugCGO,1.0,0.0,0.0);
      CGOBegin(DebugCGO,GL_LINES);
      CGOVertexv(DebugCGO,v2);
      add3f(perp0,v2,vert);
      CGOVertexv(DebugCGO,vert);
      
      CGOColor(DebugCGO,0.0,1.0,1.0);
      CGOVertexv(DebugCGO,v3);
      add3f(perp1,v3,vert);
      CGOVertexv(DebugCGO,vert);
      CGOEnd(DebugCGO);
    }
#endif
    
    if((dp = dot_product3f(perp0,perp1))>-0.5F)
      return 0;
    
    
    dev_2 = wt*dev/2.0F;
    (*strain) += abs_dev;
    if(actual>R_SMALL8) { /* nonoverlapping */

      cross_product3f(perp0,perp1,dir);
      sign = dot_product3f(axis,dir);
      
      if(sign<0.0F)
        sc=wt*(fabs(dp)-0.5);
      else
        sc=wt*(0.5-fabs(dp));
      
      scale3f(perp0,sc,push0);
      scale3f(perp1,sc,push1);
      add3f(push0,push1,balance);
      scale3f(balance,-0.5,balance);

#if 0
      {
        float vert[3];
      /* debuggin */

        CGOColor(DebugCGO,1.0,0.5,0.0);
        CGOBegin(DebugCGO,GL_LINES);
        CGOLinewidth(DebugCGO,3.0);
        
        /* draw from */
        
        CGOVertexv(DebugCGO,v0);
        add3f(v0,push0,vert);
        CGOVertexv(DebugCGO,vert);
        
        /* draw to */
        
        CGOVertexv(DebugCGO,v1);
        add3f(v1,push1,vert);
        CGOVertexv(DebugCGO,vert);

        CGOVertexv(DebugCGO,v2);
        add3f(v2,balance,vert);
        CGOVertexv(DebugCGO,vert);

        CGOVertexv(DebugCGO,v3);
        add3f(v3,balance,vert);
        CGOVertexv(DebugCGO,vert);
        
        CGOEnd(DebugCGO);        
        
      }
#endif

      add3f(d0to1,push0,d0to1);
      add3f(d1to0,push1,d1to0);
      subtract3f(d2to3,push0,d2to3);
      subtract3f(d3to2,push1,d3to2);

    } else { /* overlapping, so just push along X */
      d0to1[0]-=dev_2;
      d1to0[0]+=dev_2;
    }
    return 1;
  }
  return 0;
}

#endif


#if 0
 { int a4,b4,n3;
  float *v4;

          /* add additional long-range distance restraints into object (INEFFICIENT) */

          /* b1-b0-b2-b3-b4 */

          for(b0=0;b0<obj->NAtom;b0++) {
            n0 = obj->Neighbor[b0]+1;
            while(obj->Neighbor[n0]>=0) {
              b1 = obj->Neighbor[n0];
              n1 = obj->Neighbor[b0]+1;
              while(obj->Neighbor[n1]>=0) {
                b2 = obj->Neighbor[n1];
                if(b1!=b2) {
                  n2 =  obj->Neighbor[b2]+1;
                  while(obj->Neighbor[n2]>=0) {
                    b3 = obj->Neighbor[n2];
                    if(b3!=b0) {
                      n3 =  obj->Neighbor[b3]+1;
                      while(obj->Neighbor[n3]>=0) {
                        b4 = obj->Neighbor[n3];
                        if(b2!=b4) {
                          
                          if(obj->DiscreteFlag) {
                            if((cs==obj->DiscreteCSet[b0])&&
                               (cs==obj->DiscreteCSet[b1])&&
                               (cs==obj->DiscreteCSet[b2])&&
                               (cs==obj->DiscreteCSet[b3])&&
                               (cs==obj->DiscreteCSet[b4])
                               ) {
                              a0=obj->DiscreteAtmToIdx[b0];
                              a1=obj->DiscreteAtmToIdx[b1];
                              a2=obj->DiscreteAtmToIdx[b2];
                              a3=obj->DiscreteAtmToIdx[b3];
                              a4=obj->DiscreteAtmToIdx[b4];
                            } else {
                              a0=-1;
                              a1=-1;
                              a2=-1;
                              a3=-1;
                              a4=-1;
                            }
                          } else {
                            a0=cs->AtmToIdx[b0];
                            a1=cs->AtmToIdx[b1];
                            a2=cs->AtmToIdx[b2];
                            a3=cs->AtmToIdx[b3];
                            a4=cs->AtmToIdx[b4];
                          }
                          if((a0>=0)&&(a1>=0)&&(a2>=0)&&(a3>=0)&&(a4>=0)) {
                            v0 = cs->Coord+3*a0;
                            v1 = cs->Coord+3*a1;
                            v2 = cs->Coord+3*a2;
                            v3 = cs->Coord+3*a3;
                            v4 = cs->Coord+3*a4;
                            d = diff3f(v0,v1)+diff3f(v0,v2)+diff3f(v2,v3)+diff3f(v3,v4);
                            ShakerAddDistCon(I->Shaker,b0,b1,d,cShakerDistLimit,1.0F); 
                          }   
                        }
                        n3+=2;
                      }                
                    }
                    n2+=2;
                  }
                }
                n1+=2;
              }
              n0+=2;
            }
          }
 }
#endif
