/*	$Id: axis.h,v 1.2 2013/02/28 21:25:01 lombard Exp $	*/

/*
 * axis.h--
 *   structure Axis contain information about axes (both for zoomed
 *   traces and traces in main window).
 *
 * Copyright (c) 1992-3 Andrew K. Yu, Lawrence Berkeley Laboratory
 *      and University of California, Berkeley.
 * All rights reserved.
 */

#ifndef AXIS_H
#define AXIS_H

typedef struct {
    /* Y-axis */
    float vs;	    /* vertical scale (1 pixel to trace value) */
    float vsmag;    /* mag factor for vs */
    /* these are set in wave.c in */
    /* demean_trace               */
    float ymax, ymin;	/* converted to float */
    float y1, y2;       /* y1 and y2 are ymax and ymin */
    int y0;	    /* origin on y axis (in pixel) */
    /* calculated in axis.c        */

    /* T-axis */
    float hs;	    /* horizontal scale (1 pixel to how many samples) */
    int ix1, ix2;   /* sample indices of the left and right ends of axis */
} Axis;


#endif
