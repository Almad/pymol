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
#ifndef _H_ObjectMolecule
#define _H_ObjectMolecule

#include"os_python.h"

#include"PyMOLObject.h"
#include"AtomInfo.h"
#include"Vector.h"
#include"Color.h"
#include"Symmetry.h"
#include"Raw.h"

#define cKeywordAll "all"
#define cKeywordNone "none"
#define cKeywordSame "same"
#define cKeywordCenter "center"
#define cKeywordOrigin "origin"

#define cUndoMask 0x7

typedef struct {
  int index[2];
  int order;
  int id;
  int stereo; /* to preserve 2D rep */
} BondType;

typedef struct ObjectMoleculeBPRec {
  int *dist;
  int *list;
  int n_atom;
} ObjectMoleculeBPRec;

typedef struct ObjectMolecule {
  CObject Obj;
  struct CoordSet **CSet;
  int NCSet;
  struct CoordSet *CSTmpl; /* template for trajectories, etc.*/
  BondType *Bond;
  AtomInfoType *AtomInfo;
  int NAtom;
  int NBond;
  int DiscreteFlag,NDiscrete;
  int *DiscreteAtmToIdx;
  struct CoordSet **DiscreteCSet;
  int CurCSet;
  int SeleBase; /* for internal usage by  selector & only valid during selection process */
  CSymmetry *Symmetry;
  int *Neighbor;
  float *UndoCoord[cUndoMask+1];
  int UndoState[cUndoMask+1];
  int UndoNIndex[cUndoMask+1];
  int UndoIter;
  CGO *UnitCellCGO;
  int BondCounter;
  int AtomCounter;
  struct CSculpt *Sculpt;
} ObjectMolecule;

typedef struct ObjectMoleculeOpRec {
  unsigned int code;
  Vector3f v1,v2;
  int cs1,cs2;
  int i1,i2,i3,i4,*vc1,*i1VLA,*ii1,*vp1;
  float f1,f2,*f1VLA,*f2VLA,*ff1;
  double d[3][3];
  float *vv1,*vv2;
  char *charVLA;
  char *s1;
  ObjectMolecule **obj1VLA;
  AtomInfoType *ai;
  float ttt[16],*mat1;
  int nvv1,nvv2;
} ObjectMoleculeOpRec;

typedef struct {
  float maxAngle;
  float maxDistAtMaxAngle;
  float maxDistAtZero;
  float power_a,power_b;
  float factor_a, factor_b; /* 0.5/(maxAngle^power_a), 0.5/(maxAngle^power_b)) */
  float cone_dangle;
} HBondCriteria;

typedef struct {
  int is_pqr_file;
} PDBInfoRec;

/* these four letter code are left over from an 
   earlier multicharacter constant implementation
   and should be replaced with something more verbose */

#define OMOP_PDB1 1
#define OMOP_AVRT 2
#define OMOP_SFIT 3
#define OMOP_COLR 4
#define OMOP_VISI 5
#define OMOP_TTTF 6
#define OMOP_ALTR 7
#define OMOP_CSOC 8
#define OMOP_SUMC 9
#define OMOP_VERT 10
#define OMOP_SVRT 11
#define OMOP_MOME 12
#define OMOP_INVA 13
#define OMOP_MDST 14
#define OMOP_MNMX 15
#define OMOP_AlterState 16
#define OMOP_Flag 17
#define OMOP_LABL 18
#define OMOP_Identify    19
#define OMOP_Remove 20
#define OMOP_Protect 21
#define OMOP_Mask 22
#define OMOP_AddHydrogens 23
#define OMOP_SetB 24
#define OMOP_SaveUndo 25
#define OMOP_CountAtoms 26
#define OMOP_Cartoon 27
#define OMOP_Index 28
#define OMOP_PhiPsi 29
#define OMOP_SingleStateVertices 30
#define OMOP_IdentifyObjects 31
#define OMOP_FlagSet 32
#define OMOP_FlagClear 33
#define OMOP_PrepareFromTemplate 34
#define OMOP_SetGeometry 35
#define OMOP_CSetSumVertices 36
#define OMOP_CSetMoment 37
#define OMOP_CSetMinMax 38
#define OMOP_CSetIdxGetAndFlag 39
#define OMOP_CSetIdxSetFlagged 40
#define OMOP_GetObjects 41
#define OMOP_CSetMaxDistToPt 42
#define OMOP_MaxDistToPt 43
#define OMOP_CameraMinMax 44
#define OMOP_CSetCameraMinMax 45
#define OMOP_GetChains 46
#define OMOP_Spectrum 47
#define OMOP_GetBFactors 48
#define OMOP_GetOccupancies 49
#define OMOP_GetPartialCharges 50
#define OMOP_StateVRT 51
#define OMOP_CheckVis 52
#define OMOP_OnOff 53

#include"CoordSet.h"

typedef struct {
  ObjectMolecule *trg_obj,*mbl_obj; /* target and mobile objects */
  int *trg_vla,*mbl_vla;
  int n_pair;
} ObjMolPairwise;

void ObjMolPairwiseInit(ObjMolPairwise *pairwise);
void ObjMolPairwisePurge(ObjMolPairwise *pairwise);


/* Metaphorics Annotated PDB stuff */

typedef struct M4XBondType { /* now used for non-bonds as well as h-bonds */
  int atom1;
  int atom2;
  int strength;
} M4XBondType;

typedef struct {
  WordType name;
  int *site, n_site;
  int *ligand, n_ligand;
  int *water, n_water;
  M4XBondType *hbond,*nbond;
  int n_hbond,n_nbond;
} M4XContextType;

typedef struct {
  char target[ObjNameMax];
  int n_point;
  int *id_at_point;
  float *fitness;
} M4XAlignType;

typedef struct {
  int annotated_flag;
  int invisible;
  int n_context;
  M4XContextType *context;
  int xname_flag;
  char xname[ObjNameMax];
  M4XAlignType *align;
} M4XAnnoType;

void M4XAnnoInit(M4XAnnoType *m4x);
void M4XAnnoPurge(M4XAnnoType *m4x);

void M4XAlignInit(M4XAlignType *align);
void M4XAlignPurge(M4XAlignType *align);

void ObjectMoleculeM4XAnnotate(ObjectMolecule *I,M4XAnnoType *m4x,char *script_file,int match_colors,
                               int nbr_sele);

/* */
int ObjectMoleculeIsAtomBondedToName(ObjectMolecule *obj,int a0,char *name);
void ObjectMoleculeOpRecInit(ObjectMoleculeOpRec *op);
int ObjectMoleculeNewFromPyList(PyObject *list,ObjectMolecule **result);
PyObject *ObjectMoleculeAsPyList(ObjectMolecule *I);
int ObjectMoleculeGetSerial(ObjectMolecule *I);
int ObjectMoleculeSetStateTitle(ObjectMolecule *I,int state,char *text);
char *ObjectMoleculeGetStateTitle(ObjectMolecule *I,int state);

void ObjectMoleculeFree(ObjectMolecule *I); /* only for friends of ObjectMolecule */

ObjectMolecule *ObjectMoleculeNew(int discreteFlag);
void ObjectMoleculeSort(ObjectMolecule *I);
ObjectMolecule *ObjectMoleculeCopy(ObjectMolecule *obj);
void ObjectMoleculeFixChemistry(ObjectMolecule *I, int sele1, int sele2);

ObjectMolecule *ObjectMoleculeLoadXYZFile(ObjectMolecule *obj,char *fname,int frame,int discrete);
ObjectMolecule *ObjectMoleculeLoadPDBFile(ObjectMolecule *obj,char *fname,int frame,int discrete,M4XAnnoType *m4x,PDBInfoRec *pdb_info);
ObjectMolecule *ObjectMoleculeLoadPMOFile(ObjectMolecule *obj,char *fname,int frame,int discrete);
ObjectMolecule *ObjectMoleculeLoadMOLFile(ObjectMolecule *obj,char *fname,int frame,int discrete);
ObjectMolecule *ObjectMoleculeLoadMMDFile(ObjectMolecule *obj,char *fname,
                                          int frame,char *sepPrefix,int discrete);
ObjectMolecule *ObjectMoleculeLoadTOPFile(ObjectMolecule *obj,char *fname,int frame,int discrete);
ObjectMolecule *ObjectMoleculeLoadChemPyModel(ObjectMolecule *I,PyObject *model,int frame,int discrete);

ObjectMolecule *ObjectMoleculeLoadTRJFile(ObjectMolecule *obj,char *fname,int frame,
                                          int interval,int average,int start,
                                          int stop,int max,char *sele,int image,
                                          float *shift);

ObjectMolecule *ObjectMoleculeLoadRSTFile(ObjectMolecule *obj,char *fname,int frame);

ObjectMolecule *ObjectMoleculeLoadCoords(ObjectMolecule *I,PyObject *coords,int frame);

ObjectMolecule *ObjectMoleculeReadPMO(ObjectMolecule *obj,CRaw *pmo,int frame,int discrete);

ObjectMolecule *ObjectMoleculeReadMOLStr(ObjectMolecule *obj,char *molstr,int frame,int discrete);
ObjectMolecule *ObjectMoleculeReadPDBStr(ObjectMolecule *obj,char *molstr,
                                         int frame,int discrete,
                                         M4XAnnoType *m4x,char *pdb_name,
                                         char **next_pdb,PDBInfoRec *pdb_info);

ObjectMolecule *ObjectMoleculeReadMMDStr(ObjectMolecule *I,char *MMDStr,int frame,int discrete);
ObjectMolecule *ObjectMoleculeReadXYZStr(ObjectMolecule *I,char *PDBStr,int frame,int discrete);

void ObjectMoleculeExtendIndices(ObjectMolecule *I);

void ObjectMoleculeInvalidate(ObjectMolecule *I,int rep,int level);

void ObjectMoleculeRenderSele(ObjectMolecule *I,int curState,int sele);

void ObjectMoleculeSeleOp(ObjectMolecule *I,int sele,ObjectMoleculeOpRec *op);

struct CoordSet *ObjectMoleculeGetCoordSet(ObjectMolecule *I,int setIndex);
void ObjectMoleculeBlindSymMovie(ObjectMolecule *I);
void ObjectMoleculeMerge(ObjectMolecule *I,AtomInfoType *ai,
                         struct CoordSet *cs,int bondSearchFlag,
                         int aic_mask);
void ObjectMoleculeUpdateNonbonded(ObjectMolecule *I);
void ObjectMoleculeUpdateNeighbors(ObjectMolecule *I);
int ObjectMoleculeMoveAtom(ObjectMolecule *I,int state,int index,float *v,int mode,int log);
int ObjectMoleculeGetAtomVertex(ObjectMolecule *I,int state,int index,float *v);
int ObjectMoleculeGetAtomIndex(ObjectMolecule *I,int sele);
int ObjectMoleculeTransformSelection(ObjectMolecule *I,int state,
                                      int sele,float *TTT,int log,char *sname);
int ObjectMoleculeDoesAtomNeighborSele(ObjectMolecule *I, int index, int sele);
void ObjectMoleculeInferChemFromNeighGeom(ObjectMolecule *I,int state);
void ObjectMoleculeInferChemForProtein(ObjectMolecule *I,int state);
void ObjectMoleculeInferChemFromBonds(ObjectMolecule *I,int state);
void ObjectMoleculePurge(ObjectMolecule *I);

int ObjectMoleculeAddBond(ObjectMolecule *I,int sele0,int sele1,int order);
int ObjectMoleculeRemoveBonds(ObjectMolecule *I,int sele1,int sele2);

void ObjectMoleculeSaveUndo(ObjectMolecule *I,int state,int log);
void ObjectMoleculeUndo(ObjectMolecule *I,int dir);
void ObjectMoleculePrepareAtom(ObjectMolecule *I,int index,AtomInfoType *ai);
void ObjectMoleculeReplaceAtom(ObjectMolecule *I,int index,AtomInfoType *ai);
void ObjectMoleculePreposReplAtom(ObjectMolecule *I,int index,AtomInfoType *ai);
void ObjectMoleculeCreateSpheroid(ObjectMolecule *I,int average);
int ObjectMoleculeSetAtomVertex(ObjectMolecule *I,int state,int index,float *v);
int ObjectMoleculeVerifyChemistry(ObjectMolecule *I);
int ObjectMoleculeFillOpenValences(ObjectMolecule *I,int index);
int ObjectMoleculeGetTotalAtomValence(ObjectMolecule *I,int atom);
int ObjectMoleculeAdjustBonds(ObjectMolecule *I,int sele0,int sele1,int mode,int order);
void ObjectMoleculeAttach(ObjectMolecule *I,int index,AtomInfoType *nai);
void ObjectMoleculeFuse(ObjectMolecule *I,int index0,ObjectMolecule *src,int index1,int mode);
void ObjectMoleculeRenameAtoms(ObjectMolecule *I,int force);
int ObjectMoleculeAreAtomsBonded(ObjectMolecule *I,int i0,int i1);
void ObjectGotoState(ObjectMolecule *I,int state);
float ObjectMoleculeGetAvgHBondVector(ObjectMolecule *I,int atom,int state,float *v);
int ObjectMoleculeCheckBondSep(ObjectMolecule *I,int a0,int a1,int dist);
int ObjectMoleculeGetPhiPsi(ObjectMolecule *I,int ca,float *phi,float *psi,int state);
void ObjectMoleculeGetAtomSele(ObjectMolecule *I,int index, char *buffer);
void ObjectMoleculeGetAtomSeleFast(ObjectMolecule *I,int index, char *buffer);
void ObjectMoleculeGetAtomSeleLog(ObjectMolecule *I,int index, char *buffer,int quote);
int ObjectMoleculeMultiSave(ObjectMolecule *I,char *fname,int state,int append);
void ObjectMoleculeUpdateIDNumbers(ObjectMolecule *I);

void ObjectMoleculeSculptImprint(ObjectMolecule *I,int state);
float ObjectMoleculeSculptIterate(ObjectMolecule *I,int state,int n_cycle);
void ObjectMoleculeSculptClear(ObjectMolecule *I);
int ObjectMoleculeGetBondPaths(ObjectMolecule *I,int atom, int max, ObjectMoleculeBPRec *bp);
int ObjectMoleculeInitBondPath(ObjectMolecule *I,ObjectMoleculeBPRec *bp);
int ObjectMoleculePurgeBondPath(ObjectMolecule *I,ObjectMoleculeBPRec *bp);
int ObjectMoleculeGetBondPath(ObjectMolecule *I,int atom,int max,ObjectMoleculeBPRec *bp);
int ***ObjectMoleculeGetBondPrint(ObjectMolecule *I,int max_bond,int max_type,int *dim);

int ObjectMoleculeConnect(ObjectMolecule *I,BondType **bond,AtomInfoType *ai,struct CoordSet *cs,int searchFlag);
float ObjectMoleculeGetMaxVDW(ObjectMolecule *I);
int ObjectMoleculeFindOpenValenceVector(ObjectMolecule *I,int state,
                                        int index,float *v,float *seek);
int ObjectMoleculeGetCheckHBond(ObjectMolecule *don_obj,
                                int don_atom,
                                int don_state,
                                ObjectMolecule *acc_obj,
                                int acc_atom,
                                int acc_state,
                                HBondCriteria *hbc);
void ObjectMoleculeInitHBondCriteria(HBondCriteria *hbc);
int ObjectMoleculeConvertIDsToIndices(ObjectMolecule *I,int *id,int n_id);

#define cObjectMoleculeDummyOrigin 1
#define cObjectMoleculeDummyCenter 2

ObjectMolecule *ObjectMoleculeDummyNew(int mode);
void ObjectMoleculeDummyUpdate(ObjectMolecule *I,int mode);

/* internal to ObjectMolecule */

struct CoordSet *ObjectMoleculePDBStr2CoordSet(char *buffer,
                                               AtomInfoType **atInfoPtr,
                                               char **restart_model,
                                               char *segi_override,
                                               M4XAnnoType *m4x,
                                               char *pdb_name,
                                               char **next_pdb,
                                               PDBInfoRec *pdb_info);

/* legacy binary file suppoort */

typedef struct {
  int index[2];
  int order;
} BondType068;

typedef struct {
  int index[2];
  int order;
  int stereo;
} BondType083;

#endif




