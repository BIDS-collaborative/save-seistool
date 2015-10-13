/*	$Id: triplet.h,v 1.2 2013/02/28 21:24:56 lombard Exp $	*/

/*
 * triplet.h--
 *    Triplet is used to handle 3 component data. 
 *
 * Copyright (c) 1992-3 Andrew K. Yu, Lawrence Berkeley Laboratory
 *      and University of California, Berkeley.
 * All rights reserved.
 */
#ifndef TRIPLET_H
#define TRIPLET_H

#include "time.h"
#include "trvltime.h"

/* A number to put in the uncertainty field until we know how to
 * calculate something meaningful. */
#define DEF_UNCERTAINTY 1.0

/*
 * Indices of normally-aligned traces within a triplet.
 * These define the order that traces within a triplet are displayed.
 * If you change this order, be sure you check the logic in arrange_triplet()
 * to make sure it works with the new order.
 */
#define TRC_Z 0
#define TRC_Y 1
#define TRC_X 2

typedef struct triplet_ {
    struct trace_ *first_trc; /* pointer to the first trace in the triplet. *
			      * This is useful since a triplet may contain *
			      * less than three traces, so one or more of  *
			      * the x, y, z pointers below might be empty. */
    int numtrc;		/* number of traces in the triplet */
    int rotated;           /* has the triple been rotated    *
			    *  0: not rotated, but could be  *
			    *  1: has been rotated           *
			    * -1: cannot be rotated          */
	
    struct trace_ *trc[3];  /* the traces making up this triplet */
    char dir[3];        /* direction names of the three traces */

    STI_TIME  stime,	/* earliest (start) time of the 3 traces */
	  etime,	/* latest (end) time of the 3 traces */
	  sovrlap,	/* start time when 3 traces start to overlap */
	  eovrlap;	/* end time when 3 traces stop to overlap */

    /* info associated with each triplet */
    TtInfo *ttinfo;	   /* trvltime curves */
    float   rot_theta;	   /* synthetic azimuth of the 'N' component *
			    * that is, sta_rotation + synth rotation angle */
    float   rot_unc;       /* uncertainty of the rotation angle */
    float   sta_rotation;  /* true azimuth of the `N' component instrument */
} Triplet;

#endif TRIPLET_H
