/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1987 Gary W. Ng
**********/

#include "ngspice.h"
#include <stdio.h>
#include "mesdefs.h"
#include "cktdefs.h"
#include "fteconst.h"
#include "iferrmsg.h"
#include "noisedef.h"
#include "suffix.h"

/*
 * MESnoise (mode, operation, firstModel, ckt, data, OnDens)
 *    This routine names and evaluates all of the noise sources
 *    associated with MESFET's.  It starts with the model *firstModel and
 *    traverses all of its insts.  It then proceeds to any other models
 *    on the linked list.  The total output noise density generated by
 *    all of the MESFET's is summed with the variable "OnDens".
 */

extern void   NevalSrc();
extern double Nintegrate();

int
MESnoise (mode, operation, genmodel, ckt, data, OnDens)
    int mode;
    int operation;
    GENmodel *genmodel;
    CKTcircuit *ckt;
    register Ndata *data;
    double *OnDens;
{
    MESmodel *firstModel = (MESmodel *) genmodel;
    register MESmodel *model;
    register MESinstance *inst;
    char name[N_MXVLNTH];
    double tempOnoise;
    double tempInoise;
    double noizDens[MESNSRCS];
    double lnNdens[MESNSRCS];
    int i;

    /* define the names of the noise sources */

    static char *MESnNames[MESNSRCS] = {       /* Note that we have to keep the order */
	"_rd",              /* noise due to rd */        /* consistent with thestrchr definitions */
	"_rs",              /* noise due to rs */        /* in MESdefs.h */
	"_id",              /* noise due to id */
	"_1overf",          /* flicker (1/f) noise */
	""                  /* total transistor noise */
    };

    for (model=firstModel; model != NULL; model=model->MESnextModel) {
	for (inst=model->MESinstances; inst != NULL; inst=inst->MESnextInstance) {
	    if (inst->MESowner != ARCHme) continue;

	    switch (operation) {

	    case N_OPEN:

		/* see if we have to to produce a summary report */
		/* if so, name all the noise generators */

		if (((NOISEAN*)ckt->CKTcurJob)->NStpsSm != 0) {
		    switch (mode) {

		    case N_DENS:
			for (i=0; i < MESNSRCS; i++) {
			    (void)sprintf(name,"onoise_%s%s",inst->MESname,MESnNames[i]);


data->namelist = (IFuid *)trealloc((char *)data->namelist,(data->numPlots + 1)*sizeof(IFuid));
if (!data->namelist) return(E_NOMEM);
		(*(SPfrontEnd->IFnewUid))(ckt,
			&(data->namelist[data->numPlots++]),
			(IFuid)NULL,name,UID_OTHER,(void **)NULL);
				/* we've added one more plot */


			}
			break;

		    case INT_NOIZ:
			for (i=0; i < MESNSRCS; i++) {
			    (void)sprintf(name,"onoise_total_%s%s",inst->MESname,MESnNames[i]);


data->namelist = (IFuid *)trealloc((char *)data->namelist,(data->numPlots + 1)*sizeof(IFuid));
if (!data->namelist) return(E_NOMEM);
		(*(SPfrontEnd->IFnewUid))(ckt,
			&(data->namelist[data->numPlots++]),
			(IFuid)NULL,name,UID_OTHER,(void **)NULL);
				/* we've added one more plot */


			    (void)sprintf(name,"inoise_total_%s%s",inst->MESname,MESnNames[i]);

			    /*
			    OUTname(name,SV_INPUT_NOISE_V_SQ);
			    data->numPlots += 2; 
			    */

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
		    NevalSrc(&noizDens[MESRDNOIZ],&lnNdens[MESRDNOIZ],
				 ckt,THERMNOISE,inst->MESdrainPrimeNode,inst->MESdrainNode,
				 model->MESdrainConduct * inst->MESarea);

		    NevalSrc(&noizDens[MESRSNOIZ],&lnNdens[MESRSNOIZ],
				 ckt,THERMNOISE,inst->MESsourcePrimeNode,inst->MESsourceNode,
				 model->MESsourceConduct * inst->MESarea);

		    NevalSrc(&noizDens[MESIDNOIZ],&lnNdens[MESIDNOIZ],
				 ckt,THERMNOISE,inst->MESdrainPrimeNode,
				 inst->MESsourcePrimeNode,
                                 (2.0/3.0 * fabs(*(ckt->CKTstate0 + inst->MESgm))));

		    NevalSrc(&noizDens[MESFLNOIZ],(double*)NULL,ckt,
				 N_GAIN,inst->MESdrainPrimeNode, inst->MESsourcePrimeNode,
				 (double)0.0);
		    noizDens[MESFLNOIZ] *= model->MESfNcoef * 
				 exp(model->MESfNexp *
				 log(MAX(fabs(*(ckt->CKTstate0 + inst->MEScd)),N_MINLOG))) /
				 data->freq;
		    lnNdens[MESFLNOIZ] = 
				 log(MAX(noizDens[MESFLNOIZ],N_MINLOG));

		    noizDens[MESTOTNOIZ] = noizDens[MESRDNOIZ] +
						    noizDens[MESRSNOIZ] +
						    noizDens[MESIDNOIZ] +
						    noizDens[MESFLNOIZ];
		    lnNdens[MESTOTNOIZ] = 
				 log(MAX(noizDens[MESTOTNOIZ], N_MINLOG));

		    *OnDens += noizDens[MESTOTNOIZ];

		    if (data->delFreq == 0.0) { 

			/* if we haven't done any previous integration, we need to */
			/* initialize our "history" variables                      */

			for (i=0; i < MESNSRCS; i++) {
			    inst->MESnVar[LNLSTDENS][i] = lnNdens[i];
			}

			/* clear out our integration variables if it's the first pass */

			if (data->freq == ((NOISEAN*)ckt->CKTcurJob)->NstartFreq) {
			    for (i=0; i < MESNSRCS; i++) {
				inst->MESnVar[OUTNOIZ][i] = 0.0;
				inst->MESnVar[INNOIZ][i] = 0.0;
			    }
			}
		    } else {   /* data->delFreq != 0.0 (we have to integrate) */
			for (i=0; i < MESNSRCS; i++) {
			    if (i != MESTOTNOIZ) {
				tempOnoise = Nintegrate(noizDens[i], lnNdens[i],
				      inst->MESnVar[LNLSTDENS][i], data);
				tempInoise = Nintegrate(noizDens[i] * data->GainSqInv ,
				      lnNdens[i] + data->lnGainInv,
				      inst->MESnVar[LNLSTDENS][i] + data->lnGainInv,
				      data);
				inst->MESnVar[LNLSTDENS][i] = lnNdens[i];
				data->outNoiz += tempOnoise;
				data->inNoise += tempInoise;
				if (((NOISEAN*)ckt->CKTcurJob)->NStpsSm != 0) {
				    inst->MESnVar[OUTNOIZ][i] += tempOnoise;
				    inst->MESnVar[OUTNOIZ][MESTOTNOIZ] += tempOnoise;
				    inst->MESnVar[INNOIZ][i] += tempInoise;
				    inst->MESnVar[INNOIZ][MESTOTNOIZ] += tempInoise;
                                }
			    }
			}
		    }
		    if (data->prtSummary) {
			for (i=0; i < MESNSRCS; i++) {     /* print a summary report */
			    data->outpVector[data->outNumber++] = noizDens[i];
			}
		    }
		    break;

		case INT_NOIZ:        /* already calculated, just output */
		    if (((NOISEAN*)ckt->CKTcurJob)->NStpsSm != 0) {
			for (i=0; i < MESNSRCS; i++) {
			    data->outpVector[data->outNumber++] = inst->MESnVar[OUTNOIZ][i];
			    data->outpVector[data->outNumber++] = inst->MESnVar[INNOIZ][i];
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
            

