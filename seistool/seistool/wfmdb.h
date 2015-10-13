/*	$Id: wfmdb.h,v 1.2 2013/02/28 21:24:56 lombard Exp $	*/

/*
 * wfmdb.h--
 *     waveform database header file
 *
 * Copyright (c) 1992-3 Andrew K. Yu, Lawrence Berkeley Laboratory
 *      and University of California, Berkeley.
 * All rights reserved.
 */

#ifndef WFMDB_H 
#define WFMDB_H 

#include "eqevtmgr.h"

/* holds position of indiviual trace in data file */

typedef struct wfmTupleNode_ {
    char trcName[20];

    struct _evtFile *evt;
    int	 hdr_offset;
    int	 trc_offset;

    struct wfmTupleNode_ *next;
} wfmTuple;

/* not so useful afterall: should be eliminated. */	    
typedef struct {
    int kform;
} Segy_FSI;	    /* segy file specific info */

#endif WFMDB_H 
