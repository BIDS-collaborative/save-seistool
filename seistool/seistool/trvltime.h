/*	$Id: trvltime.h,v 1.2 2013/02/28 21:24:56 lombard Exp $	*/

/*
 * trvltime.h--
 *   header for trvltime module. 
 *
 * Copyright (c) 1992-3 Andrew K. Yu, Lawrence Berkeley Laboratory
 *      and University of California, Berkeley.
 * All rights reserved.
 */

#ifndef TRVLTIME_H
#define TRVLTIME_H

#include "pick.h"
#include "time.h"

typedef struct {
    char    name[PHSIZE]; /* phase name */
    float   tt;		/* travel time (in sec.) */
    int	    x;		/* coord of the phase */
    int	    active;	/* selected */
} Phase;

typedef struct {
    float   depth;	/* depth of source */
    float   delta;	/* distance from source */
    STI_TIME ot_it;	/* origin time */

    char    *which;	/* which phases (eg. basic/all/etc.) */
    Phase   *phs;	/* an array containing phases retrieved */
    int	    numPhs;	/* number of phases retrieved */

} TtInfo;


#endif
