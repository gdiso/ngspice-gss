/*
 * ciderinp.h
 * 
 * CIDER input library header
 */
 
#ifndef _CIDERINP_H
#define _CIDERINP_H


/* externals for bdryset.c */
extern int BDRYcheck(BDRYcard *, DOMNdomain *);
extern int BDRYsetup(BDRYcard *, MESHcoord *, MESHcoord *, DOMNdomain *);

/* externals for contset.c */
extern int CONTcheck(CONTcard *);
extern int CONTsetup(CONTcard *, ELCTelectrode *);

/* externals for domnset.c */
extern int DOMNcheck(DOMNcard *, MaterialInfo *);
extern int DOMNsetup(DOMNcard *,DOMNdomain **, MESHcoord *, MESHcoord *,
                     MaterialInfo *);

/* externals for dopset.c */
extern int DOPcheck(DOPcard *, MESHcoord *, MESHcoord *);
extern int DOPsetup(DOPcard *, DOPprofile **, DOPtable **, 
                    MESHcoord *, MESHcoord *);

/* externals for elctset.c */
extern int ELCTcheck(ELCTcard *);
extern int ELCTsetup(ELCTcard *, ELCTelectrode **, MESHcoord *,
                     MESHcoord *);

/* externals for matlset.c */
extern int MATLcheck(MATLcard *);
extern int MATLsetup(MATLcard *, MaterialInfo **);

/* externals for meshset.c */
extern double *MESHmkArray(MESHcoord *, int);
extern void MESHiBounds(MESHcoord *, int *, int *);
extern void MESHlBounds(MESHcoord *, double *, double *);
extern int MESHlocate(MESHcoord *, double);
extern int MESHcheck(char, MESHcard *);
extern int MESHsetup(char, MESHcard *, MESHcoord **, int *);

/* externals for mobset.c */
extern int MOBcheck(MOBcard *, MaterialInfo *);
extern int MOBsetup(MOBcard *, MaterialInfo *);

/* externals for modlset.c */
extern int MODLcheck(MODLcard *);
extern int MODLsetup(MODLcard *);

/* externals for outpset.c */
extern int OUTPcheck(OUTPcard *);
extern int OUTPsetup(OUTPcard *);



#endif
