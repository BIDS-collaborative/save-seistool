/*	$Id: trace.h,v 1.2 2003/02/21 21:45:59 lombard Exp $	*/

/*
 *  trace.h--
 *     structure Trace contains everything about each loaded trace 
 *
 *  Copyright (c) 1992-3 Andrew K. Yu, Lawrence Berkeley Laboratory
 *       and University of California, Berkeley.
 *  All rights reserved.
 */

#ifndef TRACE_H
#define TRACE_H

#include "amplitude.h"
#include "axis.h"
#include "wave.h"
#include "pick.h"
#include "triplet.h"
#include "clientdata.h"
#include "reg_select.h"
#include "hook.h"

/*
 * Trace holds all the information the program has about each
 * trace. (It includes the scaling factors for the trace currently
 * scaled in the main trace window as well as the zoom window. The
 * scales are retained even when the traces are not currently displayed).
 *
 */
typedef struct trace_ {
    int itrc;		/* its index -- not used yet */
    int loadOrder;	/* order in which the trace is loaded in */
    char *filename;	/* the file from which the trace is loaded */

    int axis_needScale;	/* resize events occured, needs rescaling */
    Axis axis;	    	/* scaling factors in tracks window */
    Wave *wave;		/* data and info of the trace */

    int zaxis_needScale; /* resize events occured, needs rescaling */
    Axis zaxis;	    	/* scaling factors in Zoom window */

    Pick *picks;	/* picks, in arbitrary order */
    Amp *amps;          /* amplitudes, in arbitrary order */
    
    int selected;	/* selected trace */
    int mark1_idx;	/* index of right mark of selected "window" */
    int mark2_idx;	/* index of left mark of selected "window" */
    Reg_select *sel_reg; /* selection region list */ 

    Triplet *trip;	/* associated triplet */

    ClientData *client_data;	/* client data */
    Hook *hooks;	/* special procedural hooks */
} Trace;

#endif
