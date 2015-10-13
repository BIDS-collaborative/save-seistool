/*	$Id: track.h,v 1.4 2013/02/28 21:24:57 lombard Exp $	*/

/*
 * track.h--
 *    header for track.c
 *
 * Copyright (c) 1992-3 Andrew K. Yu, Lawrence Berkeley Laboratory
 *      and University of California, Berkeley.
 * All rights reserved.
 */

#ifndef TRACK_H
#define TRACK_H

#include <xview/xview.h>
#include <xview/canvas.h>


/*
 * Track contains all the information about a ``track'' in the
 * main traces window. This ADT provides information necessary for
 * displaying the trace but not the trace itself. In other words,
 * any trace can associate with the "Track".
 */
typedef struct {
    int height, width;		/* height and width of the track */
    Canvas  canvas;		/* the plot canvas */
    Canvas  TxtCanvas;		/* the label canvas */
    Window xwin;		/* the plot Window */
    Window txtwin;		/* the label Window */
    GC	gc;			/* gc for plot window */
    GC	ovrGC;			/* overlay (XOR) gc for plot window */
    GC	txtgc;			/* gc for label Window */
} Track;

#define	    TotalInitTracks	128	/* total initial number of tracks */
#define	    NumInitTracks	10	/* number of tracks displayed */

#define	    TXTCANV_WIDTH	120

#define InTrkRange(x)	((x)>=lowTrkIndex && (x)<=highTrkIndex)
#endif
