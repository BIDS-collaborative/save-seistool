/*	$Id: ztrack.h,v 1.2 2013/02/28 21:24:55 lombard Exp $	*/

/*
 * ztrack.h--
 *    contains information about zoomed traces (but is intimately
 *    related to track.h)
 *
 * Copyright (c) 1992-3 Andrew K. Yu, Lawrence Berkeley Laboratory
 *      and University of California, Berkeley.
 * All rights reserved.
 */

#ifndef ZTRACK_H
#define ZTRACK_H

#include <X11/Xlib.h>

#define NumInitZTracks	3
#define InitZCvsWidth   500
#define InitZCvsHeight  400
#define ZSCvsHeight	30

#define ZTrkLabWidth	120    /* for labels stuff (ztrklab.c) */

#define	SCRL_UP		0
#define SCRL_DOWN	1

extern int lowZTrkIndex;	/* index of first track zoomed */
extern int highZTrkIndex;	/* index of last track zoomed */
extern int NumZTracks;		/* number of zoomed traces */

extern int ZTrkHeight;		/* height of a single track */
extern int ZCvsWidth;		/* width of the zoom window */
extern int ZCvsHeight;		/* height of the zoom window */

extern int ZFactor;		/* the zoom factor */

extern int ZoomWindowMapped;	/* whether zoom window mapped */

#define InZTrkRange(x) ((x)>=lowZTrkIndex && (x)<=highZTrkIndex)

extern Window ZCvsWin;
extern GC   ZCvsGC, ZCvsXorGC;


/*  MACROS:  */
#define UpdateMarks(i) \
    if(InTrkRange(i)) UpdateZTrkMark(i, traces[i], tracks[i-lowTrkIndex])
#define UpdateAllMarks	CleanMarks
#define CleanMarks() \
   {int i; for(i=lowZTrkIndex; i <= highZTrkIndex; i++)   /* clean the marks */ \
	if(InTrkRange(i))UpdateZTrkMark(i, traces[i], tracks[i-lowTrkIndex]); }

/*
 * Time Scale stuff
 */
#define ZTSHEIGHT	25
 
#endif
