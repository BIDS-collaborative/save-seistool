/*	$Id: eqevtmgr.h,v 1.2 2013/02/28 21:25:00 lombard Exp $	*/

/*
 * eqevtmgr.h--
 *   Earthquake Events Manager header file
 *   (handles files for each event)
 *
 * Copyright (c) 1992-3 Andrew K. Yu, Lawrence Berkeley Laboratory
 *      and University of California, Berkeley.
 * All rights reserved.
 */

#ifndef EQEVTMGR_H
#define EQEVTMGR_H

#define MAXFILENAME	128

#include "types.h"   
#include "wfmdb.h"

typedef struct _evtFile {
    char name[MAXFILENAME];
    FileFormat	format;

    wfmTuple *wfmTuples;
    void *fsi;		    /* file specific info */
    
    struct _evtFile *next;
} EvtFile;

/* a doubly linked list for keeping track of events */
typedef struct _eqEvent {
    EvtFile	    *evt;
    struct _eqEvent  *prev, *next;
} EqEvent;


extern FileFormat defaultFormat;

#endif
