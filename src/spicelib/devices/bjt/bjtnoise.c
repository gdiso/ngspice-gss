/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1987 Gary W. Ng
**********/

#include "ngspice.h"
#include <stdio.h>
#include "bjtdefs.h"
#include "cktdefs.h"
#include "fteconst.h"
#include "iferrmsg.h"
#include "noisedef.h"
#include "suffix.h"

/*
 * BJTnoise (mode, operation, firstModel, ckt, data, OnDens)
 *
 *    This routine names and evaluates all of the noise sources
 *    associated with BJT's.  It starts with the model *firstModel and
 *    traverses all of its insts.  It then proceeds to any other models
 *    on the linked list.  The total output noise density generated by
 *    all of the BJT's is summed with the variable "OnDens".
 */

extern void   NevalSrc();
extern double Nintegrate();

int
BJTnoise (mode, operation, genmodel, ckt, data, OnDens)
    GENmodel *genmodel;
    int mode;
    int operation;
    CKTcircuit *ckt;
    register Ndata *data;
    double *OnDens;
{
    BJTmodel *firstModel = (BJTmodel *) genmodel;
    register BJTmodel *model;
    register BJTinstance *inst;
    char name[N_MXVLNTH];
    double tempOnoise;
    double tempInoise;
    double noizDens[BJTNSRCS];
    double lnNdens[BJTNSRCS];
    int i;

    /* define the names of the noise sources */

    static char *BJTnNames[BJTNSRCS] = {
	/* Note that we have to keep the order consistent with the
	  strchr definitions in BJTdefs.h */
	"_rc",              /* noise due to rc */
	"_rb",              /* noise due to rb */
	"_re",              /* noise due to re */
	"_ic",              /* noise due to ic */
	"_ib",              /* noise due to ib */
	"_1overf",          /* flicker (1/f) noise */
	""                  /* total transistor noise */
    };

    for (model=firstModel; model != NULL; model=model->BJTnextModel) {
	for (inst=model->BJTinstances; inst != NULL;
		inst=inst->BJTnextInstance) {
	    if (inst->BJTowner != ARCHme) continue;

	    switch (operation) {

	    case N_OPEN:

		/* see if we have to to produce a summary report */
		/* if so, name all the noise generators */

		if (((NOISEAN*)ckt->CKTcurJob)->NStpsSm != 0) {
		    switch (mode) {

		    case N_DENS:
			for (i=0; i < BJTNSRCS; i++) {
			    (void)sprintf(name,"onoise_%s%s",
				inst->BJTname,BJTnNames[i]);


			data->namelist = (IFuid *)
				trealloc((char *)data->namelist,
				(data->numPlots + 1)*sizeof(IFuid));
			if (!data->namelist) return(E_NOMEM);
			(*(SPfrontEnd->IFnewUid))(ckt,
			    &(data->namelist[data->numPlots++]),
			    (IFuid)NULL,name,UID_OTHER,(void **)NULL);
				/* we've added one more plot */
			}
			break;

		    case INT_NOIZ:
			for (i=0; i < BJTNSRCS; i++) {
			    (void)sprintf(name,"onoise_total_%s%s",
				inst->BJTname,BJTnNames[i]);

			data->namelist = (IFuid *)
				trealloc((char *)data->namelist,
				(data->numPlots + 1)*sizeof(IFuid));
			if (!data->namelist) return(E_NOMEM);
			(*(SPfrontEnd->IFnewUid))(ckt,
			    &(data->namelist[data->numPlots++]),
			    (IFuid)NULL,name,UID_OTHER,(void **)NULL);
				/* we've added one more plot */

			    (void)sprintf(name,"inoise_total_%s%s",
				inst->BJTname,BJTnNames[i]);

data->namelist = (IFuid *)trealloc((char *)data->namelist,(data->numPlots + 1)*sizeof(IFuid));
if (!data->namelist) return(E_NOMEM);
		(*(SPfrontEnd->IFnewUid))(ckt,
			&(data->namelist[data->numPlots++]),
			(IFuid)NULL,name,UID_OTHER,(void **)NULL);
				/* we've added one more plot */
			}
			break;
		    }
		}
		break;

	    case N_CALC:
		switch (mode) {

		case N_DENS:
		    NevalSrc(&noizDens[BJTRCNOIZ],&lnNdens[BJTRCNOIZ],
				 ckt,THERMNOISE,inst->BJTcolPrimeNode,inst->BJTcolNode,
				 model->BJTcollectorConduct * inst->BJTarea);

		    NevalSrc(&noizDens[BJTRBNOIZ],&lnNdens[BJTRBNOIZ],
				 ckt,THERMNOISE,inst->BJTbasePrimeNode,inst->BJTbaseNode,
				 *(ckt->CKTstate0 + inst->BJTgx));

		    NevalSrc(&noizDens[BJT_RE_NOISE],&lnNdens[BJT_RE_NOISE],
				 ckt,THERMNOISE,inst->BJTemitPrimeNode,inst->BJTemitNode,
				 model->BJTemitterConduct * inst->BJTarea);

		    NevalSrc(&noizDens[BJTICNOIZ],&lnNdens[BJTICNOIZ],
			         ckt,SHOTNOISE,inst->BJTcolPrimeNode, inst->BJTemitPrimeNode,
				 *(ckt->CKTstate0 + inst->BJTcc));

		    NevalSrc(&noizDens[BJTIBNOIZ],&lnNdens[BJTIBNOIZ],
				 ckt,SHOTNOISE,inst->BJTbasePrimeNode, inst->BJTemitPrimeNode,
				 *(ckt->CKTstate0 + inst->BJTcb));

		    NevalSrc(&noizDens[BJTFLNOIZ],(double*)NULL,ckt,
				 N_GAIN,inst->BJTbasePrimeNode, inst->BJTemitPrimeNode,
				 (double)0.0);
		    noizDens[BJTFLNOIZ] *= model->BJTfNcoef * 
				 exp(model->BJTfNexp *
				 log(MAX(fabs(*(ckt->CKTstate0 + inst->BJTcb)),N_MINLOG))) /
				 data->freq;
		    lnNdens[BJTFLNOIZ] = 
				 log(MAX(noizDens[BJTFLNOIZ],N_MINLOG));

		    noizDens[BJTTOTNOIZ] = noizDens[BJTRCNOIZ] +
						    noizDens[BJTRBNOIZ] +
						    noizDens[BJT_RE_NOISE] +
						    noizDens[BJTICNOIZ] +
						    noizDens[BJTIBNOIZ] +
						    noizDens[BJTFLNOIZ];
		    lnNdens[BJTTOTNOIZ] = 
				 log(noizDens[BJTTOTNOIZ]);

		    *OnDens += noizDens[BJTTOTNOIZ];

		    if (data->delFreq == 0.0) { 

			/* if we haven't done any previous integration, we need to */
			/* initialize our "history" variables                      */

			for (i=0; i < BJTNSRCS; i++) {
			    inst->BJTnVar[LNLSTDENS][i] = lnNdens[i];
			}

			/* clear out our integration variables if it's the first pass */

			if (data->freq == ((NOISEAN*)ckt->CKTcurJob)->NstartFreq) {
			    for (i=0; i < BJTNSRCS; i++) {
				inst->BJTnVar[OUTNOIZ][i] = 0.0;
				inst->BJTnVar[INNOIZ][i] = 0.0;
			    }
			}
		    } else {   /* data->delFreq != 0.0 (we have to integrate) */

/* In order to get the best curve fit, we have to integrate each component separately */

			for (i=0; i < BJTNSRCS; i++) {
			    if (i != BJTTOTNOIZ) {
				tempOnoise = Nintegrate(noizDens[i], lnNdens[i],
				      inst->BJTnVar[LNLSTDENS][i], data);
				tempInoise = Nintegrate(noizDens[i] * data->GainSqInv ,
				      lnNdens[i] + data->lnGainInv,
				      inst->BJTnVar[LNLSTDENS][i] + data->lnGainInv,
				      data);
				inst->BJTnVar[LNLSTDENS][i] = lnNdens[i];
				data->outNoiz += tempOnoise;
				data->inNoise += tempInoise;
				if (((NOISEAN*)ckt->CKTcurJob)->NStpsSm != 0) {
				    inst->BJTnVar[OUTNOIZ][i] += tempOnoise;
				    inst->BJTnVar[OUTNOIZ][BJTTOTNOIZ] += tempOnoise;
				    inst->BJTnVar[INNOIZ][i] += tempInoise;
				    inst->BJTnVar[INNOIZ][BJTTOTNOIZ] += tempInoise;
                                }
			    }
			}
		    }
		    if (data->prtSummary) {
			for (i=0; i < BJTNSRCS; i++) {     /* print a summary report */
			    data->outpVector[data->outNumber++] = noizDens[i];
			}
		    }
		    break;

		case INT_NOIZ:        /* already calculated, just output */
		    if (((NOISEAN*)ckt->CKTcurJob)->NStpsSm != 0) {
			for (i=0; i < BJTNSRCS; i++) {
			    data->outpVector[data->outNumber++] = inst->BJTnVar[OUTNOIZ][i];
			    data->outpVector[data->outNumber++] = inst->BJTnVar[INNOIZ][i];
			}
		    }    /* if */
		    break;
		}    /* switch (mode) */
		break;

	    case N_CLOSE:
		return (OK);         /* do nothing, the main calling routine will close */
		break;               /* the plots */
	    }    /* switch (operation) */
	}    /* for inst */
    }    /* for model */

return(OK);
}
            

