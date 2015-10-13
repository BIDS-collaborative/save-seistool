/*	$Id: types.h,v 1.1 2001/12/21 18:39:05 lombard Exp $	*/

/*
 * types.h--
 *    contains types and defines used globally
 *
 * Copyright (c) 1992-3 Andrew K. Yu, Lawrence Berkeley Laboratory
 *      and University of California, Berkeley.
 * All rights reserved.
 */

#ifndef TYPES_H
#define TYPES_H


typedef enum {
    BIS_Format,	    /* "Berkeley Institute of Seismology" format */
    TRC_Format,	    /* an obscure format used at LBL */
    SDR_Format,	    /* steim-1 compressed Seed Data Records format */
    SEGY_Format,    /* (customised) SEG-Y format used at LBL */
    SAC_Format	    /* Seismic Analysis Code (LLNL) format */
} FileFormat;

#define	MSEED_Format	SDR_Format

#define TRACK_INDEX	123		/* used with XV_KEY_DATA */

#define LEFT_BRACKET	1
#define RIGHT_BRACKET	0

#define	TS_UP		    1
#define	TS_DN		    2

#endif
