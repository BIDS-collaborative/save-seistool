#ifndef lint
static char id[] = "$Id: preview.c,v 1.2 2013/02/28 21:24:58 lombard Exp $";
#endif

/*
 * preview.c--
 *    partial loading of data files. (Not completely finished)
 *
 * Copyright (c) 1992-3 Andrew K. Yu, Lawrence Berkeley Laboratory
 *      and University of California, Berkeley.
 * All rights reserved.
 */
#include <ctype.h>
#include <stdio.h>

#include "proto.h"
#include "xv_proto.h"

static int PreviewFile(char *fname, FileFormat format, wfmTuple **wfmtup,
		       void **fsi, EvtFile *evt)
{
    int i;

    *wfmtup= *fsi= NULL;
    
    switch(format) {
	case BIS_Format: 
	    i= PreviewBIS(fname, wfmtup, fsi, evt);
	    break;
	case TRC_Format:
	/* not working completely: trcOff not done yet: */
	    i= PreviewTRC(fname, wfmtup, fsi, evt);
	    break;
	case SDR_Format:
	    i= PreviewSDR(fname, wfmtup, fsi, evt);
	    break;
	case SEGY_Format:
	    i= PreviewSEGY(fname, wfmtup, fsi, evt);
	    break;
	case SAC_Format:
/*	    i= PreviewSAC(fname, wfmtup, fsi);*/
	    i=0;
	    fprintf(stderr,"SAC preview doesn't work yet.\n");
	    break;

	default:
	    break;
    }
    return i;
}


void PreviewOneEvent(EvtFile *ls)
{
    wfmTuple *wfm;
    while(ls!=NULL) {
	char *s;
	s= ls->name;
	PreviewFile(ls->name, ls->format, &ls->wfmTuples, &ls->fsi, ls);
	wfm=ls->wfmTuples;
	while(wfm) {
	    wfm=wfm->next;
	}
	ls=ls->next;
    }
}
