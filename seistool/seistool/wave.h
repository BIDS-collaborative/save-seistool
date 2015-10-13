/*	$Id: wave.h,v 1.3 2013/02/28 21:24:56 lombard Exp $	*/

/*
 * wave.h--
 *   structure Wave contains the data and "header" for each trace.
 *   (I prefer to keep these together so that more than one wave can
 *   be associated with a trace, if desired.)
 *
 * Copyright (c) 1992-3 Andrew K. Yu, Lawrence Berkeley Laboratory
 *      and University of California, Berkeley.
 * All rights reserved.
 */

#ifndef WAVE_H
#define WAVE_H

#include    "bis3_header.h"

/*
 * Wave contains information about the trace and its raw data.
 * The data are all converted to float if not already in float.
 */
typedef struct {
    BIS3_HEADER	info;
    float *data;	/* all data will be converted to float */
    float dcOffset;	/* the d.c. offset */
} Wave;


#endif
